/* gateTypes.glsl */


//////// GATE TYPE DEFINITIONS ////////
#define G_PASSTHROUGH 0x0u

//////// GATE TYPE DEFINITIONS ////////


void processGate(
	in uint gateType, in uint inputs[3],
	inout uint internalState, out uint externalState
) {
	switch (gateType) {
		case G_PASSTHROUGH: {
			//Just pass through A. Acts as "blank" gate.
			externalState = inputs[0];
			break;
		}

		default: {
			//Do nothing.
			break;
		}
	}
}