#include "shaderAttributes.h"

#include "glDebug.h"

#include "../core/nameToIdMap.h"

#include <cstring>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>

using namespace render;

namespace {
	NameToIdMap attribNameMap;

	struct AttribDesc {
		size_t nameId;
		int location;
		int size;
		int type;

		AttribDesc() :
				nameId(0), location(-1), size(0), type(0) {
		}

		AttribDesc(const std::string & name, int location, int size, int type) :
						nameId(attribNameMap.getId(name)),
						location(location),
						size(size),
						type(type) {
		}
	};
}

struct ShaderAttributes::impl {
	int numAttributes;
	std::vector<AttribDesc> attributes;

	impl(int id) {
		int maxAttribLen;

		glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribLen);

		glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &numAttributes);

		for (int i = 0; i < numAttributes; ++i) {
			GLint size;
			GLenum type;
#ifdef WIN32
			char * attribName = static_cast<char *>(_malloca(sizeof(char) * maxAttribLen));
#else
			char attribName[maxAttribLen];
#endif

			glGetActiveAttrib(id, i, maxAttribLen, nullptr, &size, &type,
					attribName);
			GLint loc = glGetAttribLocation(id, attribName);

			if (loc == -1) {
				if (strcmp(attribName, "gl_vertex") == 0) {
					loc = 0;
				} else if (strncmp(attribName, "gl_", 3) == 0) {
					continue;
				}
			}

			AttribDesc attrib(attribName, loc, size, type);
			// pad out attributes
			while (attributes.size() <= attrib.nameId) {
				attributes.emplace_back();
			}
			attributes[attrib.nameId] = attrib;
		}
	}
};

/*
 *
 */
ShaderAttributes::ShaderAttributes(int id) :
		pimpl(new impl(id)) {
}

/*
 *
 */
ShaderAttributes::~ShaderAttributes() {
}

/*
 *
 */
int ShaderAttributes::getNumAttributes() const {
	return pimpl->numAttributes;
}

/*
 *
 */
int ShaderAttributes::getAttributeIndex(const std::string & name) const {
	if (attribNameMap.contains(name) == false) {
		return -1;
	}
	auto index = attribNameMap.getId(name);
	if (pimpl->attributes.size() <= index) {
		return -1;
	}
	return pimpl->attributes[index].location;
}

/*
 *
 */
void ShaderAttributes::enable() const {
	for (auto attr : pimpl->attributes) {
		if (attr.location != -1) {
			glEnableVertexAttribArray(attr.location);
		}
	}
}

/*
 *
 */
void ShaderAttributes::disable() const {
	for (auto attr : pimpl->attributes) {
		if (attr.location != -1) {
			glDisableVertexAttribArray(attr.location);
		}
	}
}
/*
 *
 */
void ShaderAttributes::dump() const {
	for (auto attrib : pimpl->attributes) {
		if (attrib.location == -1) {
			continue;
		}
		std::cout << "\tattrib: " + attribNameMap.getName(attrib.nameId);
		std::cout << ", id: " + attrib.location;
		std::cout << ", size: " + attrib.size;
		std::cout << ", type: " + GlDebug::getType(attrib.type) << std::endl;
	}
}
