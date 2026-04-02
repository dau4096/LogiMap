/* types.h */
#ifndef TYPES_H
#define TYPES_H

#include "includes.h"


//Mirrors the #define version found in the gateTypes.glsl shader helper file.
enum GateType {
	G_BLANK = 0x0u,
	G_PASSTHROUGH = 0x1u,
	G_NOT = 0x2u
};



inline std::unordered_map<unsigned int, glm::uvec2> gatePositionMap = {}; //Maps index values to their new 2D positions.

namespace types {


struct Gate {
	unsigned int ID;
	GateType type;
	unsigned int inputs[3];

	Gate() : ID(0u), type(G_PASSTHROUGH), inputs{0u, 0u, 0u} {}
	Gate(const unsigned int id, const GateType gT, unsigned int i[3])
		: ID(id), type(gT) {
			std::copy(i, i+3, inputs); //Copy values.
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
				(inputPosition.x & 0xFFFFu) | //Former 16b
				((inputPosition.y & 0xFFFFu) << 16u) //Latter 16b
			);
		}

		return packedData;
	}
};


}


inline std::vector<std::vector<types::Gate>> gateLayers = {}; //2D map of gate layers

inline glm::uvec2 writeGateToLayer(unsigned int layer, unsigned int index, const types::Gate& gate) {
	//Small helper function.
	gateLayers[layer][index] = gate;
	glm::uvec2 uv = glm::uvec2(layer, index);
	gatePositionMap[gate.ID] = uv;
	return uv;
}


#endif