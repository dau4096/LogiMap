/* gateTypes.glsl */


//////// GATE TYPE DEFINITIONS ////////
#define G_BLANK 0x0u
#define G_TRUE 0x1u
#define G_FALSE 0x2u

#define G_PASSTHROUGH 0x3u
#define G_NOT 0x4u
//////// GATE TYPE DEFINITIONS ////////


void processGate(
	in uint gateType, in uint inputs[3],
	inout uint internalState, out uint externalState
) {
	switch (gateType) {
		case G_BLANK: {
			//Do nothing, this is invoked when there is a blank tile (uvec4(0,0,0,0))
			break;
		}

		case G_TRUE: {
			//Constant True
			externalState = 1u;
			break;
		}
		case G_FALSE: {
			//Constant False
			externalState = 0u;
			break;
		}



		//Real gates.
		case G_PASSTHROUGH: {
			//Just pass through A. Acts as a Buffer gate.
			externalState = inputs[0];
			break;
		}

		case G_NOT: {
			//Inverts A.
			externalState = 1 - inputs[0];
			break;
		}

		default: {
			//Do nothing.
			break;
		}
	}
}