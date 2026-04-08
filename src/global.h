#pragma once
#include "includes.h"
#include "constants.h"
using namespace std;


inline GLFWwindow* Window;


inline std::unordered_map<int, bool> keyMap = {
	//GLFW Enums mapped to boolean values (True if pressed.)
	{GLFW_KEY_ESCAPE, false}, //Close program
	{GLFW_KEY_SPACE, false} //Step to next tick.
};


inline unsigned int tickNumber;


inline unsigned int maxGatesInLayer;
inline unsigned int numberOfLayers;


namespace GLIndex {

//Any indices required for OpenGL stuff.
inline GLuint logiMap;
inline GLuint tickShader;

//IO data
inline GLuint IOSSBO;
inline void* IOptr;

}
