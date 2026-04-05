/* gateTypes.glsl */


//Leaves some gap for future gate types to be added between.
//////// GATE TYPE DEFINITIONS ////////
//Meta gate types
#define G_BLANK 0x00u
#define G_TRUE 0x01u
#define G_FALSE 0x02u
//Maybe?
//Could INPUT via uniforms (?) and output via CPU-side data reading? Unsure. Consider further.
#define G_INPUT 0x03u
#define G_OUTPUT 0x04u

//Basic gate types
#define G_PASSTHROUGH 0x10u
#define G_NOT 0x11u
#define G_AND 0x12u
#define G_OR 0x13u
#define G_XOR 0x14u
#define G_NAND 0x15u
#define G_NOR 0x16u
#define G_XNOR 0x17u
#define G_MUX 0x18u
#define G_DMUX 0x19u

//More complex logic components
#define G_DFF 0x20u
#define G_JK 0x21u
#define G_PULSE 0x22u
#define G_DELAY 0x23u
//////// GATE TYPE DEFINITIONS ////////


void processGate(
	in uint gateType, in uint inputs[3],
	inout uint internalState, out uint externalState
) {
	//Internal state is used to track some signal from a previous tick.
	//External state acts as an output to the gate. Will be read as inputs[i] for other gates, if needed.

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






		//Basic gates
		case G_PASSTHROUGH: {
			//Just pass through A. Acts as a Buffer gate.
			externalState = inputs[0];
			break;
		}

		case G_NOT: {
			//Inverts A.
			externalState = inputs[0] + 1u; //Inverts the lowest bit (0b00 → 0b01, 0b01 → 0b10), which is & 0x1u before being written back to the data.
			break;
		}

		case G_AND: {
			//A & B.
			externalState = inputs[0] & inputs[1];
			break;
		}

		case G_OR: {
			//A | B.
			externalState = inputs[0] | inputs[1];
			break;
		}

		case G_XOR: {
			//A ^ B
			externalState = inputs[0] ^ inputs[1];
			break;
		}

		case G_NAND: {
			//!(A & B)
			externalState = (inputs[0] & inputs[1]) + 1u; //See G_NOT for why +1.
			break;
		}

		case G_NOR: {
			//!(A | B)
			externalState = (inputs[0] | inputs[1]) + 1u; //See G_NOT for why +1.
			break;
		}

		case G_XNOR: {
			//! (A ^ B)
			externalState = (inputs[0] ^ inputs[1]) + 1u; //See G_NOT for why +1.
			break;
		}

		case G_MUX: {
			//Multiplexer. Chooses A or B based on signal C.
			externalState = (inputs[2] > 0u) ? inputs[1] : inputs[0];
			break;
		}

		case G_DMUX: {
			//De-Multiplexer. Requires 2 outputs, but this is not supported presently. Act as G_FALSE gate presently.
			externalState = 0u;
			break;
		}



		//More complex logic components
		case G_DFF: {
			//Stores A whenever B is true.
			if (inputs[1] > 0u) {internalState = inputs[0];}
			externalState = internalState;
			break;
		}


		case G_JK: {
			//J/K Flip-Flop. Sets on A, resets on B.
			if (inputs[0] > 0u) {internalState = 0u;}
			else if (inputs[1] > 0u) {internalState = 1u;}
			externalState = internalState;
			break;
		}


		case G_PULSE: {
			//Creates a "pulse" for 1 tick whenever A changes False → True.
			externalState = internalState ^ inputs[0]; //External = (Internal != A)
			internalState = inputs[0];
			break;
		}


		case G_DELAY: {
			//Outputs A from the PREVIOUS tick.
			externalState = internalState;
			internalState = inputs[0];
			break;
		}



		default: {
			//Do nothing.
			break;
		}
	}
}