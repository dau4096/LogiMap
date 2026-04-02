#pragma once
#include "includes.h"
#include "constants.h"
using namespace std;


inline GLFWwindow* Window;


inline std::unordered_map<int, bool> keyMap = {
	//GLFW Enums mapped to boolean values (True if pressed.)
	{GLFW_KEY_ESCAPE, false}, //Example
};


inline unsigned int frameNumber;


inline unsigned int maxGatesInLayer;
inline unsigned int numberOfLayers;


namespace GLIndex {

//Any indices required for OpenGL stuff.
inline GLuint logiMap;
inline GLuint tickShader;

}
