#include "shaderUniforms.h"

#include "glDebug.h"
#include "uniform.h"

#include <cassert>
#include <cstring>
#include <iostream>

#include <GL/glew.h>

using namespace render;

namespace {
	/**
	 * Used to cache uniform location for a shader
	 */
	struct UniformDesc {
		/** name of uniform */
		std::string m_name;
		/** id of uniform, this is the index into the shader uniform cache */
		size_t m_uid;
		/** location of uniform, as returned by glGetUniformLocation */
		int m_location;
		/** uniform size, for debug */
		int m_size;
		/** uniform type, for debug */
		int m_type;

		UniformDesc() :
				m_name(""), m_uid(0), m_location(-1), m_size(0), m_type(0) {
		}

		/**
		 * Constructor, nameId is from a static map of names to integers
		 *
		 * @param name
		 *            uniform name, as retrieved from glGetActiveUniform
		 * @param location
		 *            location of uniform, as returned by glGetUniformLocation
		 * @param size
		 *            uniform size, as retrieved from glGetActiveUniform
		 * @param type
		 *            uniform type, as retrieved from glGetActiveUniform
		 */
		UniformDesc(const std::string & name, size_t uid, int location, int size,
				int type) :
						m_name(name),
						m_uid(uid),
						m_location(location),
						m_size(size),
						m_type(type) {
		}
	};
}

struct ShaderUniforms::impl {
	int numUniforms;
	int numUserUniforms;
	std::vector<std::string> userUniformNames;
	std::vector<int> textureIds;
	std::vector<UniformDesc> uniforms;

	/**
	 *
	 * @param id
	 */
	impl(int id) {
		GLint maxUformLen;
		glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUformLen);
		size_t maxUid = 0;

		glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &numUniforms);
		numUserUniforms = 0;

		for (int i = 0; i < numUniforms; ++i) {
			GLint size;
			GLenum type;
#ifdef WIN32
			GLchar * uformName = static_cast<GLchar *>(_malloca(sizeof(GLchar) * maxUformLen));
#else
			GLchar uformName[maxUformLen];
#endif

			glGetActiveUniform(id, i, maxUformLen, NULL, &size, &type,
					uformName);
			GLint loc = glGetUniformLocation(id, uformName);
			assert(loc != -1);

			auto uid = Uniform::getUID(uformName);

			maxUid = std::max(maxUid, uid);

			if (strncmp(uformName, "gl_", 3) != 0) {
				numUserUniforms++;
				userUniformNames.emplace_back(std::string(uformName));
			}

			// remove array suffix, if exists
			char * suffix = strstr(uformName, "[0]");
			if (suffix != nullptr && strlen(suffix) == 3) {
				*suffix = 0;
			}

			if (size > 1) {
				// array uniform
				for (int j = 0; j < size; ++j) {
					std::string elem = std::string(uformName) + "["
							+ std::to_string(j) + "]";
					uid = Uniform::getUID(elem);
					maxUid = std::max(maxUid, uid);
					// pad out uniforms array
					while (uniforms.size() <= uid) {
						uniforms.emplace_back();
					}
					// insert uniform
					uniforms[uid] = UniformDesc(elem, uid, loc + j, 1, type);
				}
			} else {
				// pad out uniforms array
				while (uniforms.size() <= uid) {
					uniforms.emplace_back();
				}
				// insert uniform
				uniforms[uid] = UniformDesc(std::string(uformName), uid, loc,
						size, type);
			}
		}

		// setup texture ids
		textureIds.assign(maxUid + 1, -1);
		int texId = 0;
		for (const auto & u : uniforms) {
			if (u.m_location == -1) {
				continue;
			}
			if (u.m_type == GL_SAMPLER_2D || u.m_type == GL_SAMPLER_2D_SHADOW
					|| u.m_type == GL_SAMPLER_CUBE || u.m_type == GL_SAMPLER_3D) {
				textureIds[u.m_uid] = texId;
				texId++;
			} else {
				textureIds[u.m_uid] = -1;
			}
		}
	}
};

/*
 *
 */
ShaderUniforms::ShaderUniforms(int id) :
		pimpl(new impl(id)) {
}

/*
 *
 */
ShaderUniforms::~ShaderUniforms() {
}

/*
 *
 */
int ShaderUniforms::getUniformLocation(size_t uid, int type) const {
	if (pimpl->uniforms.size() <= uid) {
		return -1;
	}
	const auto & uform = pimpl->uniforms[uid];
	if (uform.m_location == -1) {
		return -1;
	}
	if (type != uform.m_type
			&& (type != GL_SAMPLER_2D_SHADOW || uform.m_type != GL_SAMPLER_2D)) {
		std::cerr << "Uniform type mismatch for '" << uform.m_name << "', got '"
				<< GlDebug::getType(type) << "' require '"
				<< GlDebug::getType(uform.m_type) << "'" << std::endl;
		assert(false);
		return -1;
	}
	return uform.m_location;
}

int ShaderUniforms::getTextureId(size_t uid) const {
	return pimpl->textureIds[uid];
}

int ShaderUniforms::getNumUniforms() const {
	return pimpl->numUniforms;
}

int ShaderUniforms::getNumUserUniforms() const {
	return pimpl->numUserUniforms;
}

const std::vector<std::string> & ShaderUniforms::getUserUniformNames() const {
	return pimpl->userUniformNames;
}

void ShaderUniforms::bindTextureIds() const {
	for (const auto & u : pimpl->uniforms) {
		if (u.m_location != -1 && pimpl->textureIds[u.m_uid] > -1) {
			glUniform1i(u.m_location, pimpl->textureIds[u.m_uid]);
		}
	}
}

/*
 *
 */
void ShaderUniforms::dump() const {
	for (auto uniform : pimpl->uniforms) {
		if (uniform.m_location == -1) {
			continue;
		}
		std::cout << "\tuniform: " << uniform.m_name;
		std::cout << ", id: " << uniform.m_location;
		std::cout << ", size: " << uniform.m_size;
		std::cout << ", type: " << GlDebug::getType(uniform.m_type)
				<< std::endl;
	}
}
