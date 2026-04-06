/* hdl.cpp */

#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <iostream>

#include "types.h"
#include "utils.h"
#include "logic.h"



const std::regex chipRegex = std::regex(R"(([A-Z][a-zA-Z0-9_]+)\(([a-zA-Z_]+=[a-zA-Z_01]+(,\s?)?)*\);)");
const std::regex argRegex = std::regex(R"(([a-zA-Z0-9_]+)=([a-zA-Z0-9_]+))");
const std::regex ioDefRegex = std::regex(R"(chip\s+[a-z0-9_]+\s*\{\s*in\s+([a-z0-9_]+(?:\s*,\s*[a-z0-9_]+)*);\s*out\s+([a-z0-9_]+(?:\s*,\s*[a-z0-9_]+)*);)", std::regex::icase);
const std::regex ioArgRegex = std::regex(R"([a-zA-Z0-9_]+)");
const std::regex dirRegex = std::regex(R"((HDL\/[a-zA-Z0-9_.]+\/))");
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



void addIOGates(
	const std::vector<std::string>& inputs,
	const std::vector<std::string>& outputs
) {
	//Add the relevant I/O gates.
	//[TEMP] set all in to G_FALSE and don't set out. Later replace with proper G_INPUT/G_OUTPUT gates when added.
	for (const std::string& in : inputs) {
		logic::addGate(G_FALSE, in); //Add input gates
	}
}



namespace HDL {


bool getIODefs(
	const std::string& filePath,
	std::vector<std::string>& inputs,
	std::vector<std::string>& outputs
) {
	std::string source;
	try {
		source = utils::readFile(filePath);
	} catch (const std::exception&) {
		std::cerr << "[HDL_LOAD_FAIL] Could not find file: \"" + filePath + "\"" << std::endl;
		return false; //File does not exist, return failure.
	}

	std::smatch IOmatch;
	if (std::regex_search(source, IOmatch, ioDefRegex)) {
		std::string inputDefs = IOmatch[1].str();
		std::string outputDefs = IOmatch[2].str();

		std::sregex_iterator itIn = std::sregex_iterator(inputDefs.begin(), inputDefs.end(), ioArgRegex);
		std::sregex_iterator itOut = std::sregex_iterator(outputDefs.begin(), outputDefs.end(), ioArgRegex);

		std::sregex_iterator endIn, endOut;

		//For def in each line, format them the same as the other names, so they can be properly referenced.
		for (; itIn!=endIn; itIn++) {inputs.push_back(filePath + ".0" + ":" + (*itIn)[0].str());}
		for (; itOut!=endOut; itOut++) {outputs.push_back(filePath + ".0" + ":" + (*itOut)[0].str());}
		return true;

	} else {return false; /* Failed to find CHIP I/O, Required else simulating is useless. */}
}



void getArgs(
	std::unordered_map<std::string, std::string>& chipArgs,
	const std::smatch& chipMatch, const std::string& prefix,
	const std::unordered_map<std::string, std::string>& externArgMap
) {
	//Read the inputs/outputs from the Regex match.
	std::string call = chipMatch[0].str();
	std::sregex_iterator itArg = std::sregex_iterator(call.begin(), call.end(), argRegex);
	std::sregex_iterator end;

	//Iterate through arguments
	for (; itArg!=end; itArg++) {
		const std::smatch& argMatch = *itArg;
		std::string argName = argMatch[1].str();
		std::string argValue = argMatch[2].str();
		std::cout << argName;
		auto itThis = externArgMap.find(argValue);
		if (itThis != externArgMap.end()) {
			//Replacement exists.
			chipArgs[argName] = itThis->second; //Just use the parent chip's name for it.
			std::cout << " Using previous: " << itThis->second << std::endl;
		} else {
			chipArgs[argName] = prefix + ":" + argValue; //Add a prefix so the same name in multiple files doesn't get "shared" unintentionally.
			std::cout << " Using new: " << (prefix + ":" + argValue) << std::endl;
		}
	}
}



bool parseRecurse(
	const std::string& filePath, unsigned int callNumber,
	std::unordered_set<std::string> visited, //Don't reference (instead copy), so the same chip *can* be called in multiple different recurse branches, but not in the same branch.
	const std::unordered_map<std::string, std::string>& arguments //Map to convert internal I/O arg names to external, for this call.
) {
	//Read values from this source data, then recurse into other chip calls.
	//Default case is either no chips inside to process (empty chip) or all chips are base types (from GateType.)
	std::string source;
	try {
		source = utils::readFile(filePath);
	} catch (const std::exception&) {
		std::cerr << "[HDL_LOAD_FAIL] Could not find file: \"" + filePath + "\"" << std::endl;
		return false; //File does not exist, return failure.
	}
	visited.insert(filePath);

	//For each chip call in the file.
	std::sregex_iterator itCall = std::sregex_iterator(source.begin(), source.end(), chipRegex);
	std::sregex_iterator end;

	std::vector<types::Chip> chipsToProcess = {};

	for (; itCall!=end; itCall++) {
		//For each match in the source (Each call such like `And(a=a, b=b, out=c)`)..
		const std::smatch& chipMatch = *itCall;

		//Get inputs/outputs
		std::unordered_map<std::string, std::string> chipArgs;
		getArgs(
			chipArgs, chipMatch,
			filePath + "." + std::to_string(callNumber++), //Prefix, to stop multiple different gate calls accidently sharing the same IO names.
			arguments
		);
		
		//Check if its a default chip inside the LogiMap (Like And, Or, DFF etc.)
		const std::string chipName = chipMatch[1].str();
		auto itName = defaultChipNames.find(chipName);
		if (itName != defaultChipNames.end()) {
			//Must be a default chip.
			std::cout << "Adding: " << chipName << std::endl;
			logic::addGateIO(itName->second, chipArgs);
		} else {
			//User defined chip, recurse into the relevant named file.
			std::string newFilePath;
			std::smatch dirMatch;
			std::cout << "Recursing on " << chipName << std::endl;
			if (std::regex_search(filePath, dirMatch, dirRegex)) {
				newFilePath = dirMatch[1].str() + chipName + ".hdl";
			} else {
				//Unknown file format. Just try in HDL/
				newFilePath = "HDL/" + chipName + ".hdl";
			}

			if (visited.contains(newFilePath)) {
				std::cerr << "Recursive call of " << newFilePath << " found." << std::endl;
				return false; //No infinite loops of chips.
			}
			chipsToProcess.push_back(types::Chip(
				newFilePath, chipArgs
			));
			for (const auto [key, value] : chipArgs) {
				std::cout << key << ": " << value << std::endl;
			}
		}
	}

	for (const types::Chip& chip : chipsToProcess) {
		bool success = parseRecurse(
			chip.path, callNumber, visited, chip.args
		);
		if (!success) {return false; /* Failed to parse. */}
	}

	return true;
}


void parse(const std::string filePath) {
	//Read some file, and split it into its calls to other chips.
	//logic::addGate(G_TRUE, "true"); logic::addGate(G_FALSE, "false"); //Base true/false gates.
	const std::string fullFilePath = "HDL/" + filePath;
	std::unordered_map<std::string, std::string> arguments; //Blank, no need to match to some parent chip's values.

	//Read in/out lines in `CHIP` definition.
	//Add to arguments, to define their "link" name, out of the logiMap.
	std::vector<std::string> inputs, outputs;
	bool ioSuccess = getIODefs(
		fullFilePath, inputs, outputs
	);
	if (!ioSuccess) {
		//Failed to load I/O defs
		return;
	}

	addIOGates(inputs, outputs);

	//Read chips called in `PARTS:` section.
	std::unordered_set<std::string> visited = {}; //Set of chips already found, to stop infinite loops.
	bool parseSuccess = parseRecurse(fullFilePath, 0u, visited, arguments);
	if (!parseSuccess) {
		//Failed to load HDL system.
		return;
	}


	logic::createGates();
	logic::structureLogic();
}


}