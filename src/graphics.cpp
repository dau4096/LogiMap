/* graphics.cpp */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <iostream>
#include <iomanip>
#include <regex>
#include <filesystem>

#include "includes.h"
#include "types.h"
#include "global.h"
#include "utils.h"


void APIENTRY openGLErrorCallback(
		GLenum source,
		GLenum type, GLuint id,
		GLenum severity,
		GLsizei length, const GLchar* message,
		const void* userParam
	) {
	/*
	Nicely formatted callback from;
	[https://learnopengl.com/In-Practice/Debugging]
	*/
	if ((id == 131169u) || (id == 131185u) || (id == 131218u) || (id == 131204u)) {return; /* Ignored warning IDs that are not errors */}

	std::cout << "---------------" << std::endl << "Debug message (" << id << ") | " << message << std::endl;

	switch (source)
	{
		case GL_DEBUG_SOURCE_API:             {std::cout << "Source: API"; break;}
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   {std::cout << "Source: Window System"; break;}
		case GL_DEBUG_SOURCE_SHADER_COMPILER: {std::cout << "Source: Shader Compiler"; break;}
		case GL_DEBUG_SOURCE_THIRD_PARTY:     {std::cout << "Source: Third Party"; break;}
		case GL_DEBUG_SOURCE_APPLICATION:     {std::cout << "Source: Application"; break;}
		case GL_DEBUG_SOURCE_OTHER:           {std::cout << "Source: Other"; break;}
	} std::cout << std::endl;

	switch (type)
	{
		case GL_DEBUG_TYPE_ERROR:               {std::cout << "Type: Error"; break;}
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {std::cout << "Type: Deprecated Behaviour"; break;}
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  {std::cout << "Type: Undefined Behaviour"; break;} 
		case GL_DEBUG_TYPE_PORTABILITY:         {std::cout << "Type: Portability"; break;}
		case GL_DEBUG_TYPE_PERFORMANCE:         {std::cout << "Type: Performance"; break;}
		case GL_DEBUG_TYPE_MARKER:              {std::cout << "Type: Marker"; break;}
		case GL_DEBUG_TYPE_PUSH_GROUP:          {std::cout << "Type: Push Group"; break;}
		case GL_DEBUG_TYPE_POP_GROUP:           {std::cout << "Type: Pop Group"; break;}
		case GL_DEBUG_TYPE_OTHER:               {std::cout << "Type: Other"; break;}
	} std::cout << std::endl;
	
	switch (severity)
	{
		case GL_DEBUG_SEVERITY_HIGH:         {std::cout << "Severity: high"; break;}
		case GL_DEBUG_SEVERITY_MEDIUM:       {std::cout << "Severity: medium"; break;}
		case GL_DEBUG_SEVERITY_LOW:          {std::cout << "Severity: low"; break;}
		case GL_DEBUG_SEVERITY_NOTIFICATION: {std::cout << "Severity: notification"; break;}
	} std::cout << std::endl << std::endl;

#ifdef PAUSE_ON_OPENGL_ERROR
		utils::pause();
#endif
}





static unsigned int lineNumberAt(const std::string& s, size_t pos) {
	//Find [#line] number from position
	return std::count(s.begin(), s.begin() + pos, '\n');
}

std::string preprocessIncludes(const std::string& source, const std::string& currentFile) {
	std::regex includeRegex = std::regex(R"(^\s*#include\s*<([^>]+)>)", std::regex_constants::multiline);

	std::string result;
	std::sregex_iterator it = std::sregex_iterator(source.begin(), source.end(), includeRegex);
	std::sregex_iterator end;

	size_t lastPos = 0;
	for (; it!=end; it++) {
		const std::smatch& match = *it;

		//Copy text before include
		result.append(source.substr(lastPos, match.position() - lastPos));

		std::string includeFile = match[1].str();
		std::string includePath = "src/shaders/" + includeFile + ".glsl";

		std::string includedSource = utils::readFile(includePath);

		unsigned int includeLine = lineNumberAt(source, match.position());

	#ifdef LINE_DIRECTIVE_STRING
		//Can be format `#line [lnNum] [srcFile]`
		result += "#line 1 \"src/shaders/"+includeFile+".glsl\"\n"+includedSource+"\n"+"#line "+std::to_string(includeLine+1u)+" \""+currentFile+"\"\n";
	#else
		//Must be of format `#line [lnNum]`
		result += "#line 1 \n"+includedSource+"\n"+"#line "+std::to_string(includeLine+1u)+" \n";
	#endif
		
		lastPos = match.position() + match.length();
	}

	// Append remaining source
	result.append(source.substr(lastPos));

	return result;
}



GLuint compileShader(GLenum shaderType, string filePath) {
	std::string source = utils::readFile(filePath);
	source = preprocessIncludes(source, filePath);
	const char* src = source.c_str();

	//Create a shader id
	GLuint shader = glCreateShader(shaderType);
	if (shader == 0) {
		utils::raise("Error: Failed to create shader.");
		return 0;
	}

	//Attach the shader src
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	

	//Errorcheck
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		if (!utils::isConsoleVisible()) {
			utils::showConsole();
		}
		char infolog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infolog);
		utils::raise("Error: Shader compilation failed;\n" + string(infolog));
	}

	return shader;
}




namespace uniforms {

//Uniforms; [Many overloads]
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, bool value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform1i(location, value);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, GLuint value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform1ui(location, value);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, int value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform1i(location, value);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, float value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform1f(location, value);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::ivec2 value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform2i(location, value.x, value.y);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::vec2 value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform2f(location, value.x, value.y);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::ivec3 value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform3i(location, value.x, value.y, value.z);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::vec3 value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform3f(location, value.x, value.y, value.z);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::ivec4 value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform4i(location, value.x, value.y, value.z, value.w);
	}
}
static inline void bindUniformValue(GLuint shaderProgram, const GLchar* uniformName, glm::vec4 value) {
	GLuint location = glGetUniformLocation(shaderProgram, uniformName);
	if (location >= 0) {
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}
}

}





namespace graphics {

GLFWwindow* initialiseWindow(glm::ivec2 resolution, const char* title) {
	if (!glfwInit()) {
		utils::raise("Failed to initialize GLFW");
		return nullptr;
	}


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, display::OPENGL_VERSION_MAJOR);  //OpenGL major ver (4)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, display::OPENGL_VERSION_MINOR);  //OpenGL minor ver (6)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  //Use Core (not ES)


	GLFWwindow* Window = glfwCreateWindow(resolution.x, resolution.y, title, NULL, NULL);
	if (!Window) {
		glfwTerminate();
		utils::raise("Failed to create GLFW window");
		return nullptr;
	}
	glfwMakeContextCurrent(Window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		utils::raise("Failed to initialize GLEW.");
	}

	return Window;
}




GLuint createComputeShader(std::string compShaderName) {
	GLuint computeShader = compileShader(GL_COMPUTE_SHADER, "src/shaders/" + compShaderName);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, computeShader);
	glLinkProgram(shaderProgram);

	GLint success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		if (!utils::isConsoleVisible()) {
			utils::showConsole();
		}
		char infolog[512];
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infolog);
		utils::raise("Error: Compute shader program linking failed:\n" + std::string(infolog));
	}

	glDeleteShader(computeShader);

	return shaderProgram;
}






//// TEXTURES ////
void saveImage(GLuint textureID, glm::ivec2 resolution, bool silent=false) {
	std::vector<unsigned char> pixels(resolution.x * resolution.y * 3u);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_flip_vertically_on_write(true);

	std::filesystem::path dirName = std::filesystem::path("saved.images");
	std::filesystem::create_directories(dirName);

	std::string timeStr = utils::getTimestamp();
	std::filesystem::path imagePath = dirName / (timeStr + ".png");

	stbi_write_png(
		imagePath.string().c_str(),
		resolution.x, resolution.y,
		3u, pixels.data(), resolution.x*3u
	);

	if (!silent) {std::cout << "Successfully saved image as : [" << imagePath << "]" << std::endl;}
}



GLuint createGLImage2D(
	unsigned int width, unsigned int height,
	GLint internalFormat=GL_RGBA32F,
	GLint samplingType=GL_NEAREST,
	GLint edgeSampling=GL_REPEAT
) {
	
	if ((width < 1u) || (height < 1u)) {return -1; /* Invalid size */}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, samplingType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, samplingType);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

//// TEXTURES ////




void writeGatesToMap() {
	unsigned int layerIndex = 0u;
	unsigned int gateIndex;
	std::vector<glm::uvec4> mapData(maxGatesInLayer * numberOfLayers); //Reserve space. Intentionally, uvec4(0,0,0,0) is valid as a "blank space" gate.

	for (const std::vector<types::Gate>& layer : gateLayers) {
		gateIndex = 0u;
		for (const types::Gate& gate : layer) {
			mapData[(layerIndex * maxGatesInLayer) + gateIndex] = gate.pack();
			gateIndex++;
		}
		layerIndex++;
	}

	glBindTexture(GL_TEXTURE_2D, GLIndex::logiMap);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, 0, 0,
		maxGatesInLayer, numberOfLayers, //W, H
		GL_RGBA_INTEGER, GL_UNSIGNED_INT, //Format, Type (GL_RGBA32UI)
		mapData.data()
	);
	glBindTexture(GL_TEXTURE_2D, 0);

	utils::GLErrorcheck("LogiMap data assignment", true); //Old basic debugging
}


void prepareOpenGL(void) {
	//OpenGL setup;

	maxGatesInLayer = 0u;
	for (const std::vector<types::Gate>& layer : gateLayers) {maxGatesInLayer = glm::max(maxGatesInLayer, (unsigned int)(layer.size()));}
	numberOfLayers = gateLayers.size();

	std::cout << maxGatesInLayer << " " << numberOfLayers << std::endl;
	if ((maxGatesInLayer == 0u) || (numberOfLayers == 0u)) {return; /* Nothing more to be done. */}

	GLIndex::logiMap = createGLImage2D(
		maxGatesInLayer,     //Highest number of gates in any layer.
		numberOfLayers,     //Number of layers total.
		GL_RGBA32UI,       //uint32_t types.
		GL_NEAREST,       //No interp.
		GL_CLAMP_TO_EDGE //Edges repeat.
	);
	utils::GLErrorcheck("LogiMap allocation", true); //Old basic debugging

	writeGatesToMap();


	GLIndex::tickShader = createComputeShader("tick.comp");


	//Debug settings
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openGLErrorCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	utils::GLErrorcheck("Initialisation", true); //Old basic debugging
}

}




namespace tick {

void run(void) {

	glUseProgram(GLIndex::tickShader);
	unsigned int Xdispatch = (maxGatesInLayer + 31u) / 32u; //Local size is (32,1,1)

#ifdef CONCURRENT_LAYERS
	
	uniforms::bindUniformValue(GLIndex::tickShader, "baseLayerIndex", (GLuint)(0u));
	uniforms::bindUniformValue(GLIndex::tickShader, "numGatesInLayer", (GLuint)(maxGatesInLayer));
	glBindImageTexture(0, GLIndex::logiMap, 0, GL_FALSE, 0, GL_READ_WRITE,  GL_RGBA32UI);
	glDispatchCompute(
		Xdispatch,
		numberOfLayers,
		1u
	);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);

#else

	for (GLuint i=0u; i<numberOfLayers; i++) {
		uniforms::bindUniformValue(GLIndex::tickShader, "baseLayerIndex", i);
		uniforms::bindUniformValue(GLIndex::tickShader, "numGatesInLayer", (GLuint)(gateLayers[i].size()));
		glBindImageTexture(0, GLIndex::logiMap, 0, GL_FALSE, 0, GL_READ_WRITE,  GL_RGBA32UI);
		glDispatchCompute(
			Xdispatch,
			1, 1
		);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
	}

#endif

	glUseProgram(0);

}

}