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



void testLogic() {

#ifdef TEST_PASSTHROUGH
	//Passes some value through multiple layers.
	types::Gate gStart = types::Gate(
		0u, G_TRUE, 0u, 0u, 0u //Index, Type, A, B, C
	);
	writeGateToLayer(0u, gStart); //Layer 0
	
	for (unsigned int i=1u; i<4u; i++) {
		//3 extra passthroughs.
		types::Gate gateI = types::Gate(
			i, G_PASSTHROUGH, i-1u, 0u, 0u //Index, Type, A, B, C
		);
		writeGateToLayer(i, gateI); //Layer i
	}
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
