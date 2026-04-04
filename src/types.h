/* types.h */
#ifndef TYPES_H
#define TYPES_H

#include "includes.h"


//Mirrors the #define version found in the gateTypes.glsl shader helper file.
enum GateType {
	//Meta gate types
	G_BLANK = 0x0u,
	G_TRUE = 0x1u,
	G_FALSE = 0x2u,

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

namespace types {


struct Gate {
	unsigned int ID;
	GateType type;
	unsigned int inputs[3];
	unsigned int numInputs;

	Gate() : ID(0u), type(G_PASSTHROUGH), inputs{0u, 0u, 0u} {}
	Gate(const unsigned int id, const GateType gT, unsigned int i[3])
		: ID(id), type(gT) {
			std::copy(i, i+3, inputs); //Copy values.
			numInputs = 0u;
			for (const unsigned int& i : inputs) {if (i != INVALID_ID) {numInputs++;}}
		}
	Gate(const unsigned int id, const GateType gT, unsigned int i0, unsigned int i1, unsigned int i2)
		: ID(id), type(gT), inputs{i0, i1, i2} {
			numInputs = 0u;
			for (const unsigned int& i : inputs) {if (i != INVALID_ID) {numInputs++;}}
		}

	glm::uvec4 pack() const {
		//Pack metadata.
		glm::uvec4 packedData = glm::uvec4(
			((GLuint)(type) & 0xFFu) << 2u, //Metadata
			0u, 0u, 0u //3 Inputs
		);

		//Pack inputs
		for (GLuint i=0u; i<3u; i++) {
			//Indices 1-3 inclusive (YZW)
			glm::uvec2 inputPosition = gatePositionMap[inputs[i]];
			packedData[i+1] = (
				(inputPosition.x & 0xFFFFu) | //Former 16b is index in layer
				((inputPosition.y & 0xFFFFu) << 16u) //Latter 16b is layer index
			);
		}

		return packedData;
	}
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