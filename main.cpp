/* main.cpp */

#include <iostream>

#include "src/includes.h"
#include "src/types.h"
#include "src/global.h"
#include "src/utils.h"
#include "src/graphics.h"
#include "src/hdl.h"




#ifdef TICK_STEP
bool prevStep = false;
#endif
void handleInputs() {
	glfwPollEvents();

	//Get keyboard inputs for this frame
	for (std::pair<int, bool> pair : keyMap) {
		int keyState = glfwGetKey(Window, pair.first);
		if (keyState == GLFW_PRESS) {keyMap[pair.first] = true;}
		else if (keyState == GLFW_RELEASE) {keyMap[pair.first] = false;}
	}

}




int main() {
	try { //Catch exceptions

#ifdef __WIN32
	SetConsoleOutputCP(65001); //CP_UTF8, Windows.
#else
	#pragma execution_character_set("utf-8") //Linux.
#endif

	Window = graphics::initialiseWindow(glm::ivec2(1, 1), "LogiMap/Main");
	utils::GLErrorcheck("Window Creation", true);

	//Read HDL file
	HDL::parse("HDL/FullAdderSplit.hdl");


	//Initialise.
	graphics::prepareOpenGL();
#ifdef HAS_WINDOW
	//Resize window
	glm::ivec2 res = glm::ivec2(maxGatesInLayer, numberOfLayers) * display::WINDOW_SCALING_FACTOR;
	glViewport(0, 0, res.x, res.y);
	glfwSetWindowSize(Window, res.x, res.y);
#endif



#ifdef TICK_STEP
	std::cout << "Press [SPACE] to advance simulation by 1 tick." << std::endl;
#endif


	tickNumber = 0u;
	while (!glfwWindowShouldClose(Window)) {
		double frameStart = glfwGetTime();
		handleInputs();
		if (keyMap[GLFW_KEY_ESCAPE]) {break; /* Quit Immediately, ESC pressed. */}


	#ifdef TICK_STEP
		bool step = (keyMap[GLFW_KEY_SPACE] && !prevStep);
	#else
		bool step = (tickNumber < constants::NUMBER_OF_TICKS_TO_SIM);
	#endif
		tick::run(step);



	#ifdef HAS_WINDOW
		glfwSwapBuffers(Window);
	#endif

		prevStep = keyMap[GLFW_KEY_SPACE];
		tickNumber++;
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
