#pragma once

#include <string>

class GlDebug {
public:
	static const std::string & getType(int type);

	static void printOpenGLError();

	static void printShaderInfoLog(const std::string & tag, int shader);

	static void printProgramInfoLog(const std::string & tag, int program);
};
