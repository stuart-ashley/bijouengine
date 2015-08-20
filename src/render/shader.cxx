#include "shader.h"

#include "glDebug.h"
#include "shaderAttributes.h"
#include "shaderUniforms.h"

#include <cassert>
#include <iostream>

#include <GL/glew.h>

using namespace render;

namespace {
	std::string getWord(const std::string & str, size_t & idx) {
		size_t n = str.size();
		std::string word = "";
		// skip non word characters
		for (; idx < n; ++idx) {
			char c = str[idx];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
					|| (c >= '0' && c <= '9') || c == '_') {
				break;
			}
		}
		// word until non word character
		for (; idx < n; ++idx) {
			char c = str[idx];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
					|| (c >= '0' && c <= '9') || c == '_') {
				word += c;
			} else {
				break;
			}
		}
		return word;
	}

	void compileShader(int id, const std::string & name,
			const std::string & src) {
		const GLchar * str = src.c_str();
		GLint strSize = static_cast<GLint>(src.size());

		glShaderSource(id, 1, &str, &strSize);

		GlDebug::printShaderInfoLog(name, id);

		glCompileShader(id);

		GlDebug::printShaderInfoLog(name, id);
	}

	std::string concatNames(const std::shared_ptr<Program> & vp,
			const std::shared_ptr<Program> & gp,
			const std::shared_ptr<Program> & fp) {
		if (gp != nullptr) {
			return vp->getName() + " " + gp->getName() + " " + fp->getName();
		} else {
			return vp->getName() + " " + fp->getName();
		}
	}
}

struct Shader::impl {
	std::string m_name;
	unsigned m_sid;
	std::string m_defs;
	std::shared_ptr<Program> m_vp;
	std::shared_ptr<Program> m_gp;
	std::shared_ptr<Program> m_fp;
	int m_maxInstances;
	std::unique_ptr<ShaderAttributes> m_attributes;
	std::unique_ptr<ShaderUniforms> m_uniforms;
	int m_glShader;

	impl(int sid, const std::string & defs, const std::shared_ptr<Program> & vp,
			const std::shared_ptr<Program> & gp,
			const std::shared_ptr<Program> & fp, int maxInstances) :
					m_name(concatNames(vp, gp, fp)),
					m_sid(sid),
					m_defs(defs),
					m_vp(vp),
					m_gp(gp),
					m_fp(fp),
					m_maxInstances(maxInstances),
					m_glShader(0) {
	}

	void build(const std::string & vertSrc, const std::string & geomSrc,
			const std::string & fragSrc) {
		// vertex shader
		int vid = glCreateShader(GL_VERTEX_SHADER);
		GlDebug::printShaderInfoLog(m_name + " vp", vid);

		compileShader(vid, m_name + " vp", vertSrc);

		// geometry shader
		int gid = 0;
		std::string varyingMappings;
		if (geomSrc != "") {
			gid = glCreateShader(GL_GEOMETRY_SHADER_EXT);
			GlDebug::printShaderInfoLog(m_name + " gp", gid);

			compileShader(gid, m_name + " gp", geomSrc);

			size_t idx = 0;
			std::string word = getWord(geomSrc, idx);
			while (word != "") {
				if (word == "Varying" && geomSrc[idx] == ':') {
					varyingMappings += "#define " + getWord(geomSrc, idx) + " "
							+ getWord(geomSrc, idx) + "\n";
				}
				word = getWord(geomSrc, idx);
			}
		}

		// fragment shader
		int fid = glCreateShader(GL_FRAGMENT_SHADER);
		GlDebug::printShaderInfoLog(m_name + " fp", fid);

		compileShader(fid, m_name + " fp", varyingMappings + fragSrc);

		// program
		m_glShader = glCreateProgram();

		glAttachShader(m_glShader, vid);
		if (gid > 0) {
			glAttachShader(m_glShader, gid);
		}
		glAttachShader(m_glShader, fid);

		if (gid > 0) {
			std::string inputType;
			std::string outputType;

			size_t idx = 0;
			std::string word = getWord(geomSrc, idx);
			while (word != "") {
				if (word == "Input" && geomSrc[idx] == ':') {
					inputType = getWord(geomSrc, idx);
				} else if (word == "Output" && geomSrc[idx] == ':') {
					outputType = getWord(geomSrc, idx);
				}
				word = getWord(geomSrc, idx);
			}

			if (inputType == "GL_TRIANGLES") {
				glProgramParameteriEXT(m_glShader, GL_GEOMETRY_INPUT_TYPE_EXT,
						GL_TRIANGLES);
			} else if (inputType == "GL_POINTS") {
				glProgramParameteriEXT(m_glShader, GL_GEOMETRY_INPUT_TYPE_EXT,
						GL_POINTS);
			} else if (inputType == "GL_LINES") {
				glProgramParameteriEXT(m_glShader, GL_GEOMETRY_INPUT_TYPE_EXT,
						GL_LINES);
			} else {
				std::cerr << "Error: Invalid geometry shader input type"
						<< std::endl;
				assert(false);
			}
			if (outputType == "GL_TRIANGLE_STRIP") {
				glProgramParameteriEXT(m_glShader, GL_GEOMETRY_OUTPUT_TYPE_EXT,
						GL_TRIANGLE_STRIP);
			} else if (outputType == "GL_POINTS") {
				glProgramParameteriEXT(m_glShader, GL_GEOMETRY_OUTPUT_TYPE_EXT,
						GL_POINTS);
			} else if (outputType == "GL_LINE_STRIP") {
				glProgramParameteriEXT(m_glShader, GL_GEOMETRY_OUTPUT_TYPE_EXT,
						GL_LINE_STRIP);
			} else {
				std::cerr << "Error: Invalid geometry shader output type"
						<< std::endl;
				assert(false);
			}

			GLint maxGeometryOutputVertices;
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT,
					&maxGeometryOutputVertices);
			glProgramParameteriEXT(m_glShader, GL_GEOMETRY_VERTICES_OUT_EXT,
					maxGeometryOutputVertices);
		}

		glLinkProgram(m_glShader);
		GlDebug::printProgramInfoLog(m_name, m_glShader);
	}

};

Shader::Shader(int sid, const std::string & defs,
		const std::shared_ptr<Program> & vp,
		const std::shared_ptr<Program> & gp,
		const std::shared_ptr<Program> & fp, int maxInstances) :
		pimpl(new impl(sid, defs, vp, gp, fp, maxInstances)) {
}

/*
 *
 */
Shader::~Shader() {
}

/**
 *
 */
void Shader::bind() {
	assert(pimpl->m_glShader != 0);

	glUseProgram(pimpl->m_glShader);
	pimpl->m_attributes->enable();
	pimpl->m_uniforms->bindTextureIds();
}

/**
 *
 */
void Shader::bindEmpty() {
	glUseProgram(0);
}

/**
 *
 */
void Shader::build() {
	if (pimpl->m_gp != nullptr) {
		pimpl->build(pimpl->m_defs + pimpl->m_vp->getSource(),
				pimpl->m_defs + pimpl->m_gp->getSource(),
				pimpl->m_defs + pimpl->m_fp->getSource());
	} else {
		pimpl->build(pimpl->m_defs + pimpl->m_vp->getSource(), "",
				pimpl->m_defs + pimpl->m_fp->getSource());
	}

	pimpl->m_attributes = std::unique_ptr<ShaderAttributes>(
			new ShaderAttributes(pimpl->m_glShader));
	pimpl->m_uniforms = std::unique_ptr<ShaderUniforms>(
			new ShaderUniforms(pimpl->m_glShader));
}

/**
 *
 * @return  true if shader has been built, false otherwise
 */
bool Shader::built() const {
	return pimpl->m_attributes != nullptr;
}

/**
 *
 */
void Shader::dump() const {
	std::cout << "== Shader " + pimpl->m_name << std::endl;

	pimpl->m_attributes->dump();

	pimpl->m_uniforms->dump();
}

/**
 *
 * @param name
 * @return
 */
int Shader::getAttributeIndex(const std::string & name) const {
	return pimpl->m_attributes->getAttributeIndex(name);
}

/**
 *
 * @return
 */
unsigned Shader::getId() const {
	return pimpl->m_sid;
}

/**
 *
 * @return
 */
int Shader::getMaxInstances() const {
	return pimpl->m_maxInstances;
}

/**
 *
 * @return
 */
int Shader::getNumAttributes() const {
	return pimpl->m_attributes->getNumAttributes();
}

/**
 *
 * @return
 */
int Shader::getNumUniforms() const {
	return pimpl->m_uniforms->getNumUniforms();
}

/**
 *
 * @return
 */
int Shader::getNumUserUniforms() const {
	return pimpl->m_uniforms->getNumUserUniforms();
}

/**
 *
 * @param uid
 * @return
 */
size_t Shader::getTextureId(size_t uid) const {
	return pimpl->m_uniforms->getTextureId(uid);
}

/**
 *
 * @param uid
 * @param type
 * @return
 */
int Shader::getUniformLocation(size_t uid, int type) const {
	return pimpl->m_uniforms->getUniformLocation(uid, type);
}

/**
 *
 * @return
 */
const std::vector<std::string> & Shader::getUserUniformNames() const {
	return pimpl->m_uniforms->getUserUniformNames();
}

/**
 * Disable vertex attributes and vertex buffer object
 */
void Shader::unbind() {
	pimpl->m_attributes->disable();
	// disable vbo
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

