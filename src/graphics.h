/* graphics.h */
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "includes.h"
#include "global.h"
#include "utils.h"

using namespace glm;




namespace uniforms {

//Uniforms; [Many overloads]
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, bool value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, size_t value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, int value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, float value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::ivec2 value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::vec2 value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::ivec3 value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::vec3 value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::ivec4 value);
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::vec4 value);

}




namespace graphics {
	GLFWwindow* initialiseWindow(glm::ivec2 resolution, const char* title);
	GLuint createComputeShader(std::string compShaderName);


	//// TEXTURES ////
	void saveImage(GLuint textureID, bool silent=false);
	GLuint createGLImage2D(size_t width, size_t height, GLint internalFormat=GL_RGBA32F, GLint samplingType=GL_NEAREST, GLint edgeSampling=GL_REPEAT);
	//// TEXTURES ////


	void prepareOpenGL();
}


namespace tick {

	void run();

}

#endif
