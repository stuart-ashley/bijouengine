#include "vertexAttributeArray.h"

#include "shader.h"

using namespace render;

/**
 * Default constructor
 */
VertexAttributeArray::VertexAttributeArray() {
}

/**
 * Copy constructor, does a shallow copy
 */
VertexAttributeArray::VertexAttributeArray(const VertexAttributeArray & other) :
		attributes(other.attributes) {
}

/**
 *
 * @param shader
 */
void VertexAttributeArray::bind(const std::shared_ptr<Shader> & shader) {
	int nAttribs = shader->getNumAttributes();
	int nBound = 0;

	for (auto & attribute : attributes) {
		int idx = shader->getAttributeIndex(attribute.getName());

		if (idx == -1) {
			continue;
		}

		attribute.bind(idx);
		nBound++;
		if (nBound == nAttribs)
			break;
	}
}

/**
 *
 * @param attribute
 */
void VertexAttributeArray::add(const VertexAttribute & attribute) {
	attributes.emplace_back(attribute);
}

/**
 *
 * @param other
 * @return
 */
bool VertexAttributeArray::isInstanceable(
		const VertexAttributeArray & other) const {
	return attributes == other.attributes;
}

/**
 *
 * @return
 */
bool VertexAttributeArray::validate() const {
	bool valid = true;
	for (const auto & attribute : attributes) {
		valid = valid && attribute.validate();
	}
	return valid;
}
