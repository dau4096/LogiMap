#pragma once

#include "includes.h"
#include <glm/glm.hpp>




namespace constants {

	//Mathematical Constants
	constexpr float PI = 3.141593f;
	constexpr float PI2 = PI * 2.0f;
	constexpr float EXP = 2.718281f;
	constexpr float INF = std::numeric_limits<float>::infinity();

	constexpr float TO_RAD = 0.017453f;
	constexpr float TO_DEG = 57.29577f;

}

namespace display {

	//Opengl 460 core.
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 6;

}

