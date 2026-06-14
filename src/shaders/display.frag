/* uv.frag */
#version 460 core

#include <gateTypes> //For the gate #define names.

layout(binding=0) uniform usampler2D logiMap;

in vec2 fragUV;
out vec4 fragColour;


//#define SHOW_STATE_RAW
#define SHOW_TYPE


void main() {
	vec4 gateData = texture(logiMap, fragUV);
	
#ifdef SHOW_STATE_RAW
	//Just show raw data of gate.
	fragColour = vec4(gateData.xyz, 1.0f);
#else
	uint type = (uint(gateData.x) >> 2u) & 0xFFu;
	uint state = uint(gateData.x) & 0x1u;
#ifdef SHOW_TYPE
	fragColour = vec4(float(type)/64.0f, state, 0.0f, 1.0f);
#else

	//Show output states of gate.

	switch (type) {
		case G_BLANK: {
			fragColour = vec4(0.0f, 0.0f, 0.0f, 1.0f); //Completely blank.
			break;
		}
		
		//Constant gates;
		case G_TRUE: {
			fragColour = vec4(0.0f, 1.0f, 1.0f, 1.0f);
			break;
		}

		case G_FALSE: {
			fragColour = vec4(1.0f, 0.0f, 1.0f, 1.0f);
			break;
		}

		//I/O
		case G_INPUT: case G_OUTPUT: {
			fragColour = vec4(0.0f, 0.0f, 1.0f, 1.0f);
			break;
		}

		default: {
			fragColour = (state > 0u) ? vec4(0.0f, 1.0f, 0.0f, 1.0f) : vec4(1.0f, 0.0f, 0.0f, 1.0f); //Green/Red based on state.
			break;
		}
	}
#endif
#endif
}