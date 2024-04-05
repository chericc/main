#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>

void printShaderLog(GLuint shader);
void printProgramLog(int prog);
bool checkOpenGLError();

#define CHECK_OPENGL_ERROR() \
	do {\
		if (!checkOpenGLError()) {\
			std::stringstream ss;\
			ss << "Found error: ";\
			ss << "[" << __FILE__;\
			ss << " " << __FUNCTION__;\
			ss << " " << __LINE__ << "]";\
			std::cout << ss << std::endl; \
		}\
	} while(false)