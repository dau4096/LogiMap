/* hdl.cpp */

#include <regex>
#include <unordered_map>
#include <set>
#include <string>
#include <iostream>

#include "types.h"
#include "utils.h"
#include "logic.h"
#include "graphics.h"


const std::regex chipRegex = std::regex(R"(([A-Z][a-zA-Z0-9_]+)\(([a-zA-Z0-9_\[\]\.=, ]+)\);)"); //For name(key=value, key=value);
const std::regex argRegex = std::regex(R"(([a-zA-Z0-9_\[\]\.]+)\s*=\s*([a-zA-Z0-9_\[\]\.]+))"); //For key=value ^^
const std::regex ioDefRegex = std::regex(R"(chip\s+[a-z0-9_]+\s*\{\s*in\s+([a-z0-9_\[\]\.]+(?:\s*,\s*[a-z0-9_\[\]\.]+)*);\s*out\s+([a-z0-9_\[\]\.]+(?:\s*,\s*[a-z0-9_\[\]\.]+)*);)", std::regex::icase); //for IN key, key, key; OUT key, key; etc.
const std::regex ioArgRegex = std::regex(R"([a-zA-Z0-9_]+(?:\[\d+(?:\.\.\d+)?\])?)"); //For key ^^
const std::regex dirRegex = std::regex(R"((HDL[a-zA-Z0-9_.\/]*\/))"); //For getting dir from filepath
const std::regex pinRegex = std::regex(R"(([a-zA-Z0-9_]+)(?:\[(\d+)(?:\.\.(\d+))?\])?)"); //For parsing pins/buses/slices
const std::unordered_map<std::string, GateType> defaultChipNames = {
	//Meta chips
	{"Blank", G_BLANK}, {"True", G_TRUE}, {"False", G_FALSE},

	//Basic gate types
	{"Passthrough", G_PASSTHROUGH},
	{"Not", G_NOT}, {"And", G_AND}, {"Or", G_OR}, {"Xor", G_XOR},
	{"Nand", G_NAND}, {"Nor", G_NOR}, {"Xnor", G_XNOR},
	{"Mux", G_MUX}, {"DMux", G_DMUX},

	//More complex logic components
	{"DFF", G_DFF}, {"JKflipflop", G_JK},
	{"Pulse", G_PULSE}, {"Delay", G_DELAY},
};
unsigned int ID = 0u;


void addIOGates(
	const std::set<std::string>& inputs,
	const std::set<std::string>& outputs
) {
	std::cout << "Inputs: ";
	for (const std::string& in : inputs) {
		std::cout << in << ", ";
		logic::addGate(G_INPUT, constants::ROOT_PREFIX + in);
	}

	std::cout << "\nOutputs: ";
	for (const std::string& out : outputs) {
		std::cout << out << ", ";
		logic::addGate(G_OUTPUT, out, "ROOT:" + out);
	}
	std::cout << std::endl;
}



bool loadFile(const std::string& filePath, std::string& source) {
	try {
		source = utils::readFile(filePath);
	} catch (const std::exception&) {
		std::cerr << "[HDL_LOAD_FAIL] Could not find file: \"" + filePath + "\"" << std::endl;
		return false; //File does not exist, return failure.
	}
	return true;
}




//Pre-define for getArguments.
namespace HDL {
	void connectPins(
		const std::string& left,
		const std::string& right,
		const std::string& prefix,
		const std::unordered_map<std::string,std::string>& argumentMap,
		std::unordered_map<std::string,std::string>& args
	);
}

void getArguments(
	const std::string& argsStr, const std::string& prefix,
	std::unordered_map<std::string, std::string>& args, //Read `key=value` from the str.
	const std::unordered_map<std::string, std::string>& argumentMap	 //Map for parent chip's wires to this chip's internal wire names.
) {
	std::sregex_iterator itArg = std::sregex_iterator(argsStr.begin(), argsStr.end(), argRegex);
	std::sregex_iterator end;

	for (; itArg!=end; itArg++) {
		const std::smatch& argumentMatch = *itArg;
		//key=value
		std::string key = argumentMatch[1].str();
		std::string value = argumentMatch[2].str();

		HDL::connectPins(key, value, prefix, argumentMap, args);
	}
}



std::string getDirectory(const std::string& filePath) {
	std::smatch dirMatch;
	if (std::regex_search(filePath, dirMatch, dirRegex)) {
		return dirMatch[0].str(); //Return whole match.
	} else {
		return "HDL/";
	}
}



namespace HDL {

std::set<std::string> inputs;
std::set<std::string> outputs;



types::PinInfo parsePin(const std::string& text) {
	std::smatch m;

	if (!std::regex_match(text, m, pinRegex)) {
		throw std::runtime_error("Invalid pin syntax: " + text);
	}

	types::PinInfo info;
	info.name = m[1].str();

	if (m[2].matched) {
		info.hasBracket = true;
		info.first = std::stoi(m[2].str());
	}

	if (m[3].matched) {
		info.isRange = true;
		info.second = std::stoi(m[3].str());
	}

	return info;
}




//Used in the IN/OUT defs, to define a bus.
std::vector<std::string> expandDeclaration(
	const std::string& text
) {
	types::PinInfo pin = parsePin(text);

	if (!pin.hasBracket) {return {pin.name,};}

	if (pin.isRange) {
		throw std::runtime_error("Ranges not allowed in declarations: " + text);
	}


	std::vector<std::string> result;
	for (unsigned int i=0u; i<pin.first; i++) {
		result.push_back(
			pin.name + "[" + std::to_string(i) + "]"
		);
	}

	return result;
}


//Used inside the PARTS list, to index bus pins.
std::vector<std::string> expandReference(
	const std::string& text
) {
	types::PinInfo pin = parsePin(text);

	if (!pin.hasBracket) {return {pin.name,};}

	if (!pin.isRange) {
		return {pin.name + "[" + std::to_string(pin.first) + "]",};
	}


	std::vector<std::string> result;
	for (unsigned int i=pin.first; i<=pin.second; i++) {
		result.push_back(
			pin.name + "[" + std::to_string(i) + "]"
		);
	}

	return result;
}





std::vector<std::string> resolvePins(
	const std::string& text,
	const std::string& prefix,
	const std::unordered_map<std::string,std::string>& argumentMap
) {
	std::vector<std::string> pins = expandReference(text);

	for (std::string& pin : pins) {
		auto it = argumentMap.find(pin);

		if (it != argumentMap.end()) {
			pin = it->second;
		} else {
			pin = prefix + pin;
		}
	}

	return pins;
}



void connectPins(
	const std::string& left,
	const std::string& right,
	const std::string& prefix,
	const std::unordered_map<std::string,std::string>& argumentMap,
	std::unordered_map<std::string,std::string>& args
) {
	std::vector<std::string> lhs = expandReference(left);
	std::vector<std::string> rhs = resolvePins(
		right,
		prefix,
		argumentMap
	);

	if (lhs.empty()) {
		throw std::runtime_error("Invalid pin: " + left);
	}

	if (rhs.empty()) {
		throw std::runtime_error("Invalid pin: " + right);
	}

	if (lhs.size() != rhs.size()) {
		throw std::runtime_error("Bus width mismatch: " + left + " ←→ " + right);
	}

	for (unsigned int i=0u; i<lhs.size(); i++) {
		args[lhs[i]] = rhs[i];
	}
}



bool getIODefs(
	const std::string& filePath,
	std::set<std::string>& inputs,
	std::set<std::string>& outputs,
	const std::string prefix=""
) {

	std::string source;
	if (!loadFile(filePath, source)) {return false; /* Failed to load file */}

	std::smatch IOmatch;
	if (std::regex_search(source, IOmatch, ioDefRegex)) {
		std::string inputDefs = IOmatch[1].str();
		std::string outputDefs = IOmatch[2].str();

		std::sregex_iterator itIn = std::sregex_iterator(inputDefs.begin(), inputDefs.end(), ioArgRegex);
		std::sregex_iterator itOut = std::sregex_iterator(outputDefs.begin(), outputDefs.end(), ioArgRegex);
		std::sregex_iterator endIn, endOut;

		//For def in each line, format them the same as the other names, so they can be properly referenced.
		for (; itIn!=endIn; itIn++) {
			std::vector<std::string> names = expandDeclaration((*itIn)[0].str());
			for (const auto& n : names) {
				inputs.insert(prefix + n);
			}
		}
		for (; itOut!=endOut; itOut++) {
			std::vector<std::string> names = expandDeclaration((*itOut)[0].str());
			for (const auto& n : names) {
				outputs.insert(prefix + n);
			}
		}
		return true;

	} else {return false; /* Failed to find CHIP I/O, Required else simulating is useless. */}
}



bool parseRecursive(const std::string& filePath, const std::unordered_map<std::string, std::string>& argumentMap, const std::string& prefix) {
	//Recursively parse chip calls.
	std::string source;
	if (!loadFile(filePath, source)) {return false; /* Failed to load file */}

	std::vector<types::Chip> chipsToLoad = {};

	std::sregex_iterator itChip = std::sregex_iterator(source.begin(), source.end(), chipRegex);
	std::sregex_iterator end;


	//Iterate through all chips;
	for (; itChip!=end; itChip++) {
		const std::smatch& chipMatch = *itChip;
		std::string chipName = chipMatch[1].str();
		std::string chipArgsStr = chipMatch[2].str();

		std::unordered_map<std::string, std::string> chipArgsMap;
		getArguments(chipArgsStr, prefix, chipArgsMap, argumentMap);

		if (defaultChipNames.contains(chipName)) {
			//Just directly add it, it's a base defined logi component.
			logic::addGateIO(defaultChipNames.at(chipName), chipArgsMap);
		} else {
			//Must load another file to get this chip's components.
			chipsToLoad.push_back(types::Chip(
				getDirectory(filePath) + chipName + ".hdl",
				chipArgsMap
			));
		}
	}

	for (const types::Chip& chip : chipsToLoad) {
		//Recurse on this chip, to load and add it.
		if (!parseRecursive(chip.filePath, chip.args, std::to_string(ID++) + ":")) {return false; /* Somewhere in the chain failed. */}
	}


	return true;
}



void parse(const std::string filePath) {
	//Read some file, and split it into its calls to other chips.
	getIODefs(filePath, inputs, outputs);
	addIOGates(inputs, outputs);

	std::unordered_map<std::string, std::string> argumentMap; //Create mapping of inputs to parent chip's wire names. Presently just maps main file IO to itself.
	for (const std::string& in : inputs) {argumentMap[in] = constants::ROOT_PREFIX + in;}
	for (const std::string& out : outputs) {argumentMap[out] = constants::ROOT_PREFIX + out;}


	parseRecursive(filePath, argumentMap, constants::ROOT_PREFIX);


	logic::createGates();
	logic::structureLogic();

	graphics::createIOSSBO(inputs.size() + outputs.size());
}


}