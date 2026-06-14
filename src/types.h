/* types.h */
#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <unordered_map>
#include <vector>
#include "constants.h"
#include "includes.h"


//Mirrors the #define version found in the gateTypes.glsl shader helper file.
enum GateType {
	//Meta gate types
	G_BLANK = 0x00u,
	G_TRUE = 0x01u,
	G_FALSE = 0x02u,
	//IO
	G_INPUT = 0x03u,
	G_OUTPUT = 0x04u,

	//Basic gate types
	G_PASSTHROUGH = 0x10u,
	G_NOT = 0x11u,
	G_AND = 0x12u,
	G_OR = 0x13u,
	G_XOR = 0x14u,
	G_NAND = 0x15u,
	G_NOR = 0x16u,
	G_XNOR = 0x17u,
	G_MUX = 0x18u,
	G_DMUX = 0x19u,

	//More complex logic components
	G_DFF = 0x20u,
	G_JK = 0x21u,
	G_PULSE = 0x22u,
	G_DELAY = 0x23u
};
#define INVALID_ID 0xFFFFFFFFu /* Very large ID. */



inline std::unordered_map<unsigned int, glm::uvec2> gatePositionMap = {}; //Maps index values to their new 2D positions.
inline unsigned int IOcounter = 0u;


namespace types {

inline std::unordered_map<std::string, unsigned int> IOmap = {}; //Map of IO names to their indices.
inline std::vector<std::string> inputNames = {}; //Vector of input names
inline std::vector<std::string> outputNames = {}; //Vector of output names



struct Gate {
	unsigned int ID;
	GateType type;
	unsigned int inputs[3];
	unsigned int numInputs;
	std::string name; //Only used for G_INPUT/G_OUTPUT.

	Gate() : ID(0u), type(G_PASSTHROUGH), inputs{0u, 0u, 0u}, name("") {}
	Gate(const unsigned int id, const GateType gT, unsigned int i[3], const std::string n)
		: ID(id), type(gT) {
			std::copy(i, i+3, inputs); //Copy values.
			numInputs = 0u;
			for (const unsigned int& i : inputs) {if (i != INVALID_ID) {numInputs++;}}

			//Remove Root prefix if present.
			name = n;
			size_t pos = name.find(constants::ROOT_PREFIX);
			if (pos != std::string::npos) {
				name.erase(pos, constants::ROOT_PREFIX.length());
			}
		}
	Gate(const unsigned int id, const GateType gT, unsigned int i0, unsigned int i1, unsigned int i2, const std::string n)
		: ID(id), type(gT), inputs{i0, i1, i2} {
			numInputs = 0u;
			for (const unsigned int& i : inputs) {if (i != INVALID_ID) {numInputs++;}}

			//Remove Root prefix if present.
			name = n;
			size_t pos = name.find(constants::ROOT_PREFIX);
			if (pos != std::string::npos) {
				name.erase(pos, constants::ROOT_PREFIX.length());
			}
		}

	glm::uvec4 pack() const {
		//Pack metadata.
		GLuint metaData = ((GLuint)(type) & 0xFFu) << 2u;
		glm::uvec4 packedData;

		//Pack inputs.
		//Input/Output have special formats.
		if (type == G_INPUT) {
			packedData = glm::uvec4(
				metaData,
				IOcounter, //Index into the IOSSBO.
				0u, 0u //2 blank inputs (Unused)
			);
			inputNames.push_back(name);
			IOmap[name] = IOcounter++; //Increment.

		} else if (type == G_OUTPUT) {
			glm::uvec2 inputPosition = gatePositionMap[inputs[0]];
			packedData = glm::uvec4(
				metaData,
				IOcounter, //Index into the IOSSBO.
				( //Where to read the value from, to output.
					(inputPosition.x & 0xFFFFu) | //Former 16b is index in layer
					((inputPosition.y & 0xFFFFu) << 16u) //Latter 16b is layer index
				),
				0u //Blank input (Unused)
			);
			outputNames.push_back(name);
			IOmap[name] = IOcounter++; //Increment.

		} else {
			packedData = glm::uvec4(
				metaData,
				0u, 0u, 0u //3 blank inputs (Will be overwritten)
			);

			for (GLuint i=0u; i<3u; i++) {
				//Indices 1-3 inclusive (YZW)
				glm::uvec2 inputPosition = gatePositionMap[inputs[i]];
				packedData[i+1] = (
					(inputPosition.x & 0xFFFFu) | //Former 16b is index in layer
					((inputPosition.y & 0xFFFFu) << 16u) //Latter 16b is layer index
				);
			}
		}

		return packedData;
	}
};




struct GateTemplate {
	std::string name;
	GateType type;
	std::string inputs[3];
};



struct Chip {
	std::string filePath;
	std::unordered_map<std::string, std::string> args;

	Chip() : filePath(""), args() {}
	Chip(const std::string& p, const std::unordered_map<std::string, std::string>& a)
		: filePath(p), args(a) {}
};


}


extern std::vector<std::vector<types::Gate>> gateLayers; //2D map of gate layers

inline glm::uvec2 writeGateToLayer(unsigned int layer, const types::Gate& gate) {
	//Small helper function.
	if (gateLayers.size() <= layer) {gateLayers.resize(layer+1u); /* Add new empty vectors for this new layer. */}
	gateLayers[layer].push_back(gate);
	glm::uvec2 uv = glm::uvec2(gateLayers[layer].size()-1u, layer);
	gatePositionMap[gate.ID] = uv;
	return uv;
}

inline bool isGateSatisfied(unsigned int thisLayer, unsigned int otherGateID) {
	auto it = gatePositionMap.find(otherGateID);
	if (it == gatePositionMap.end()) {return false;}
	return (it->second.y < thisLayer); //Other gate's layer < this gate's layer.
}


#endif