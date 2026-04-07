/* logic.cpp */


#include <string>
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include "types.h"


const std::vector<GateType> noDepGates = {
	G_BLANK, G_FALSE, G_TRUE
};

std::unordered_map<std::string, unsigned int> nameMap = {{"", INVALID_ID},};
std::vector<types::GateTemplate> templates = {};
std::vector<types::Gate> gates = {};

namespace logic {



void structureLogic() { //Make a copy, as values [will] be removed from it in processing.
	//Loop through all of the gates, find those with "No dependencies" (Such as G_TRUE/G_FALSE etc.)
	for (unsigned int gateIndex=0u; gateIndex<gates.size();) {
		types::Gate& gate = gates[gateIndex];
		auto it = std::find(noDepGates.begin(), noDepGates.end(), gate.type);
		if (it != noDepGates.end()) {
			//Add to layer 0.
			writeGateToLayer(0, gate);
			//Remove from the vector.
			gates.erase(gates.begin() + gateIndex);
		} else {
			gateIndex++;
		}
	}

	//Now loop through until gates is empty, adding gates to new layers when all of their dependencies are fulfilled.
	unsigned int layer = 1u;
	unsigned int prevSize = gates.size();
	while (gates.size() > 0u) {
		for (unsigned int gateIndex=0u; gateIndex<gates.size();) {
			types::Gate gate = gates[gateIndex];

			bool satisfied = true;
			for (unsigned int i=0u; i<gate.numInputs; i++) {
				unsigned int gIdx = gate.inputs[i];
				satisfied &= isGateSatisfied(layer, gIdx); //Compare this layer, and the referenced gate's layer.
			}

			if (satisfied) {
				//Can add this gate.
				writeGateToLayer(layer, gate);
				gates.erase(gates.begin() + gateIndex);
			} else {
				gateIndex++;
			}

		}

		if (gates.size() == prevSize) {
			std::cerr << "[STRUCTURE_FAIL] Infinite loop detected from " << gates.size() << " logic components." << std::endl;
			break;
		}
		prevSize = gates.size();

		layer++;
	}
}


bool addGate(const GateType type, const std::string name, const std::string in0="", const std::string in1="", const std::string in2="") {
	//Add gate using string names rather than numeric IDs.
	auto it = nameMap.find(name);
	if (it != nameMap.end()) {
		//Name already taken by another gate
		std::cerr << "[STRUCTURE_FAIL] Cannot use the same name for 2 gates! \"" << name << "\" is already being used by another gate." << std::endl;
		return false; //Failure
	}

	unsigned int gateIndex = templates.size(); //New ID must be back ID + 1.
	nameMap[name] = gateIndex;

	types::GateTemplate templ;
	templ.name = name;
	templ.type = type;
	templ.inputs[0] = in0;
	templ.inputs[1] = in1;
	templ.inputs[2] = in2;
	templates.push_back(templ);

#ifdef VERBOSE
	std::cout << std::format(
		"IN: \"{}\" \"{}\" \"{}\"    OUT: \"{}\"    TYPE: {}",
		in0, in1, in2, name, std::to_string(type)
	) << std::endl;
#endif

	return true; //Success
}



bool addGateIO(
	const GateType type, const std::unordered_map<std::string, std::string>& args
) {
	//Semi-Overload of addGate() that handles named inputs/outputs.
	switch (type) {
		case G_FALSE: case G_TRUE: {
			return addGate(
				type, args.at("out"), "", ""
			);
			break;
		}

		case G_NOT: case G_PASSTHROUGH: case G_PULSE:
		case G_DELAY: case G_DFF: {
			//[Name](in=?, out=?);
			return addGate(
				type, args.at("out"), args.at("in"), "", ""
			);
			break;
		}

		case G_AND: case G_OR: case G_XOR:
		case G_NAND: case G_NOR: case G_XNOR: {
			//[Name](a=?, b=?, out=?);
			return addGate(
				type, args.at("out"), args.at("a"), args.at("b"), ""
			);
			break;
		}

		case G_MUX: {
			//[Name](a=?, b=?, sel=?, out=?);
			return addGate(
				type, args.at("out"), args.at("a"), args.at("b"), args.at("sel")
			);
			break;
		}

		case G_JK: {
			//[Name](set=?, reset=?, out=?);
			return addGate(
				type, args.at("out"), args.at("set"), args.at("reset"), ""
			);
			break;
		}

		default: {
			//Unknown type.
			std::cerr << "Unknown gate type: " << type << std::endl;
			return false;
			break;
		}
	}
	return true;
}



bool createGates() {
	//Uses map to convert named ids into actual values.

#ifdef SORT_TEMPLATES
	//Sort templates so that inputs of a gate are always added before itself.
	std::vector<std::string> definedOutputs = {"",};
	std::vector<types::GateTemplate> sortedTemplates = {};

	unsigned int numTemplates = templates.size();
	unsigned int prevSize = numTemplates;
	while (sortedTemplates.size() < numTemplates) {
		for (unsigned int templID=0u; templID<templates.size();) {
			const types::GateTemplate templ = templates[templID];

			bool satisfied = true;
			for (unsigned int i=0u; i<3; i++) {
				auto it = std::find(definedOutputs.begin(), definedOutputs.end(), templ.inputs[i]);
				satisfied &= (it != definedOutputs.end());
			}

			if (satisfied) {
				definedOutputs.push_back(templ.name);
				sortedTemplates.emplace_back(templ);
				templates.erase(templates.begin() + templID);
			} else {
				templID++;
			}
		}

		if (templates.size() == prevSize) {break; /* Infinite loop of some sort, can't define. */}
		prevSize = templates.size();
	}


	sortedTemplates.insert(sortedTemplates.end(), templates.begin(), templates.end());

	unsigned int ID = 0u;
	for (const types::GateTemplate& templ : sortedTemplates) {nameMap[templ.name] = ID++;}
#endif


	//Create each gate entry.
	unsigned int gateIndex = 0u;
#ifdef SORT_TEMPLATES
	for (const types::GateTemplate& templ : sortedTemplates) {
#else
	for (const types::GateTemplate& templ : templates) {
#endif
		unsigned int inputs[3];
		for (unsigned int i=0u; i<3u; i++) {
			auto it = nameMap.find(templ.inputs[i]);
			if (it == nameMap.end()) {
				std::cerr << "Unknown name: " << templ.inputs[i] << std::endl;
				return false;
			}
			inputs[i] = it->second;
		}
		gates.push_back(types::Gate(
			gateIndex++, templ.type, inputs
		));
	}

	return true;
}


}