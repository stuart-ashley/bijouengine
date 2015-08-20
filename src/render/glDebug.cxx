#include "glDebug.h"

#include <cassert>
#include <iostream>
#include <unordered_map>

#include <GL/glew.h>
#include <GL/glu.h>

const std::string & GlDebug::getType(int type) {
	static std::unordered_map<int, std::string> typeMap = {
			{ GL_INT, "GL_INT" }, { GL_UNSIGNED_INT, "GL_UNSIGNED_INT" }, {
					GL_FLOAT, "GL_FLOAT" }, { GL_FLOAT_VEC2, "GL_FLOAT_VEC2" },
			{ GL_FLOAT_VEC3, "GL_FLOAT_VEC3" },
			{ GL_FLOAT_VEC4, "GL_FLOAT_VEC4" },
			{ GL_FLOAT_MAT2, "GL_FLOAT_MAT2" },
			{ GL_FLOAT_MAT3, "GL_FLOAT_MAT3" },
			{ GL_FLOAT_MAT4, "GL_FLOAT_MAT4" },
			{ GL_SAMPLER_2D, "GL_SAMPLER_2D" },
			{ GL_SAMPLER_3D, "GL_SAMPLER_3D" }, { GL_SAMPLER_CUBE,
					"GL_SAMPLER_CUBE" }, { GL_SAMPLER_2D_SHADOW,
					"GL_SAMPLER_2D_SHADOW" }, { GL_TEXTURE, "GL_TEXTURE" } };

	auto it = typeMap.find(type);
	if (it == typeMap.end()) {
		std::cerr << "ERROR: Unknown type " << std::hex << type;
		assert(false);
	}
	return it->second;
}

void GlDebug::printOpenGLError() {
	GLenum err = glGetError();
	if (GL_NO_ERROR == err)
		return;
	std::cout << "OpenGL Error( " << err << ": " << gluErrorString(err) << ")";
	assert(false);
}

void GlDebug::printShaderInfoLog(const std::string & tag, int shader) {
	int infologLen = 0;
	int charsWritten = 0;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);

	printOpenGLError();

	if (infologLen <= 1) {
		return;
	}

#ifdef WIN32
	GLchar * infolog = static_cast<GLchar *>(_malloca(infologLen * sizeof(GLchar)));
#else
	GLchar infolog[infologLen];
#endif
	glGetShaderInfoLog(shader, infologLen, &charsWritten, infolog);
	std::cout << tag << std::endl << infolog << std::endl;
	printOpenGLError();
}

void GlDebug::printProgramInfoLog(const std::string & tag, int program) {
	int infologLen = 0;
	int charsWritten = 0;

	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLen);

	printOpenGLError();

	if (infologLen <= 1) {
		return;
	}

#ifdef WIN32
	GLchar * infolog = static_cast<GLchar *>(_malloca(infologLen * sizeof(GLchar)));
#else
	GLchar infolog[infologLen];
#endif
	glGetProgramInfoLog(program, infologLen, &charsWritten, infolog);
	std::cout << tag << std::endl << infolog << std::endl;
	printOpenGLError();
}
