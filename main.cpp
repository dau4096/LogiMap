/* main.cpp */

#include "src/includes.h"
#include "src/types.h"
#include "src/global.h"
#include "src/utils.h"
#include "src/graphics.h"





void handleInputs() {
	glfwPollEvents();

	//Get keyboard inputs for this frame
	for (std::pair<int, bool> pair : keyMap) {
		int keyState = glfwGetKey(Window, pair.first);
		if (keyState == GLFW_PRESS) {keyMap[pair.first] = true;}
		else if (keyState == GLFW_RELEASE) {keyMap[pair.first] = false;}
	}

}


const std::vector<GateType> noDepGates = {
	G_BLANK, G_FALSE, G_TRUE
};
void structureLogic(std::vector<types::Gate> gates) { //Make a copy, as values [will] be removed from it in processing.
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
	while (gates.size() > 0u) {
		for (unsigned int gateIndex=0u; gateIndex<gates.size();) {
			types::Gate& gate = gates[gateIndex];
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

		layer++;
	}
}



void testLogic() {

#ifdef TEST_PASSTHROUGH
	//Passes some value through multiple layers.
	std::vector<types::Gate> gates = {
		types::Gate(
			0u, G_TRUE, INVALID_ID, INVALID_ID, INVALID_ID //Index, Type, A, B, C
		),
		types::Gate(
			1u, G_PASSTHROUGH, 0u, INVALID_ID, INVALID_ID //Index, Type, A, B, C
		),
		types::Gate(
			2u, G_PASSTHROUGH, 1u, INVALID_ID, INVALID_ID //Index, Type, A, B, C
		),
		types::Gate(
			3u, G_PASSTHROUGH, 2u, INVALID_ID, INVALID_ID //Index, Type, A, B, C
		)
	};
	structureLogic(gates);
#endif


#ifdef TEST_ADDER
	//A simple 1 bit full adder circuit.
	std::vector<types::Gate> gates = {
		//Inputs
		types::Gate(
			0u, G_TRUE, INVALID_ID, INVALID_ID, INVALID_ID //Input A
		),
		types::Gate(
			1u, G_TRUE, INVALID_ID, INVALID_ID, INVALID_ID //Input B
		),
		types::Gate(
			2u, G_TRUE, INVALID_ID, INVALID_ID, INVALID_ID //Input C
		),


		//Half adder (A, B)
		types::Gate(
			3u, G_XOR, 0u, 1u, INVALID_ID
		),
		types::Gate(
			4u, G_AND, 0u, 1u, INVALID_ID
		),


		//Half adder (Q, C)
		types::Gate(
			5u, G_XOR, 3u, 2u, INVALID_ID
		),
		types::Gate(
			6u, G_AND, 3u, 2u, INVALID_ID
		),


		types::Gate(
			8u, G_PASSTHROUGH, 5u, INVALID_ID, INVALID_ID //Final XOR output beside the OR.
		),
		//Final or
		types::Gate(
			7u, G_OR, 4u, 6u, INVALID_ID
		),
	};
	structureLogic(gates);
#endif

}



int main() {
	try { //Catch exceptions

#ifdef __WIN32
	SetConsoleOutputCP(65001); //CP_UTF8, Windows.
#else
	#pragma execution_character_set("utf-8") //Linux.
#endif


#ifdef HAS_WINDOW
	glm::ivec2 res = glm::ivec2(640, 360);
#else
	glm::ivec2 res = glm::ivec2(1, 1);
#endif
	Window = graphics::initialiseWindow(res, "LogiMap/Main");
	utils::GLErrorcheck("Window Creation", true);


	//Initialise.
	testLogic();
	graphics::prepareOpenGL();


	frameNumber = 0u;
	while (!glfwWindowShouldClose(Window)) {
		double frameStart = glfwGetTime();
		handleInputs();
		if (keyMap[GLFW_KEY_ESCAPE]) {break; /* Quit Immediately, ESC pressed. */}

		tick::run();

	#ifdef HAS_WINDOW
		glfwSwapBuffers(Window);
	#endif
		frameNumber++;
	}


	//Cleanup and exit.
	glfwDestroyWindow(Window);
	glfwTerminate();
	return 0;


	//Catch exceptions.
	} catch (const std::exception& e) {
		if (!utils::isConsoleVisible()) {utils::showConsole();}
		std::cerr << "An exception was thrown: " << e.what() << std::endl;
		utils::pause();
		return -1;
	} catch (...) {
		if (!utils::isConsoleVisible()) {utils::showConsole();}
		std::cerr << "An unspecified exception was thrown." << std::endl;
		utils::pause();
		return -1;
	}
}
