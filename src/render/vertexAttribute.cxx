#include "vertexAttribute.h"

#include "glDebug.h"
#include "vertexBuffer.h"

#include "../core/color.h"
#include "../core/normal.h"
#include "../core/vec2.h"
#include "../core/vec3.h"

#include <cassert>

#include <GL/glew.h>

using namespace render;

namespace {
	int glType[] = { GL_FLOAT, GL_SHORT, GL_UNSIGNED_SHORT, GL_BYTE };
}

struct VertexAttribute::impl {
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::string name;
	int size;
	Type type;
	int offset;
	int stride;
	std::vector<float> floatBuffer;

	/*
	 *
	 */
	impl(const std::shared_ptr<VertexBuffer> & buffer, const std::string & name,
			int size, Type type, int offset, int stride) :
					vertexBuffer(buffer),
					name(name),
					size(size),
					type(type),
					offset(offset),
					stride(stride) {
		assert(buffer != nullptr);
	}

	/*
	 *
	 */
	impl(const std::string & name, const std::vector<Vec2> & array) :
					vertexBuffer(nullptr),
					name(name),
					size(2),
					type(Type::FLOAT),
					offset(0),
					stride(0) {
		floatBuffer.reserve(array.size() * 2);
		for (const auto & v : array) {
			floatBuffer.emplace_back(v.getX());
			floatBuffer.emplace_back(v.getY());
		}
	}

	/*
	 *
	 */
	impl(const std::string & name, const std::vector<Vec3> & array) :
					vertexBuffer(nullptr),
					name(name),
					size(3),
					type(Type::FLOAT),
					offset(0),
					stride(0) {
		floatBuffer.reserve(array.size() * 3);
		for (const auto & v : array) {
			floatBuffer.emplace_back(static_cast<float>(v.getX()));
			floatBuffer.emplace_back(static_cast<float>(v.getY()));
			floatBuffer.emplace_back(static_cast<float>(v.getZ()));
		}
	}

	/*
	 *
	 */
	impl(const std::string & name, const std::vector<Normal> & array) :
					vertexBuffer(nullptr),
					name(name),
					size(3),
					type(Type::FLOAT),
					offset(0),
					stride(0) {
		floatBuffer.reserve(array.size() * 3);
		for (const auto & n : array) {
			floatBuffer.emplace_back(n.getX());
			floatBuffer.emplace_back(n.getY());
			floatBuffer.emplace_back(n.getZ());
		}
	}

	/*
	 *
	 */
	impl(const std::string & name, const std::vector<Color> array) :
					vertexBuffer(nullptr),
					name(name),
					size(4),
					type(Type::FLOAT),
					offset(0),
					stride(0) {
		floatBuffer.reserve(array.size() * 4);
		for (const auto & c : array) {
			floatBuffer.emplace_back(c.getR());
			floatBuffer.emplace_back(c.getG());
			floatBuffer.emplace_back(c.getB());
			floatBuffer.emplace_back(c.getA());
		}
	}

	/*
	 *
	 */
	impl(const std::string & name, int size, const std::vector<float> array) :
					vertexBuffer(nullptr),
					name(name),
					size(size),
					type(Type::FLOAT),
					offset(0),
					stride(0),
					floatBuffer(array) {
	}
};

/**
 *
 * @param buffer
 * @param name
 * @param size
 * @param type
 * @param offset
 * @param stride
 */
VertexAttribute::VertexAttribute(const std::shared_ptr<VertexBuffer> & buffer,
		const std::string & name, int size, Type type, int offset, int stride) :
		pimpl(new impl(buffer, name, size, type, offset, stride)) {
}

/**
 *
 * @param name
 * @param array
 */
VertexAttribute::VertexAttribute(const std::string & name,
		const std::vector<Vec2> & array) :
		pimpl(new impl(name, array)) {
}

/**
 *
 * @param name
 * @param array
 */
VertexAttribute::VertexAttribute(const std::string & name,
		const std::vector<Vec3> & array) :
		pimpl(new impl(name, array)) {
}

/**
 *
 * @param name
 * @param array
 */
VertexAttribute::VertexAttribute(const std::string & name,
		const std::vector<Normal> & array) :
		pimpl(new impl(name, array)) {
}

/**
 *
 * @param name
 * @param array
 */
VertexAttribute::VertexAttribute(const std::string & name,
		const std::vector<Color> array) :
		pimpl(new impl(name, array)) {
}

/**
 *
 * @param name
 * @param size
 * @param array
 */
VertexAttribute::VertexAttribute(const std::string & name, int size,
		const std::vector<float> array) :
		pimpl(new impl(name, size, array)) {
}

/**
 *
 * @param idx
 */
void VertexAttribute::bind(int idx) {
	// shader calls glEnableVertexAttribArray on bind
	if (pimpl->vertexBuffer != nullptr) {
		pimpl->vertexBuffer->bind();
		glVertexAttribPointer(idx, pimpl->size,
				glType[static_cast<int>(pimpl->type)], true, pimpl->stride,
				reinterpret_cast<GLvoid *>(pimpl->offset));
		GlDebug::printOpenGLError();
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glVertexAttribPointer(idx, pimpl->size,
				glType[static_cast<int>(pimpl->type)], false, pimpl->stride,
				pimpl->floatBuffer.data());
		GlDebug::printOpenGLError();
	}
}

/**
 *
 * @return
 */
const std::string & VertexAttribute::getName() const {
	return pimpl->name;
}

/**
 *
 * @param other
 * @return
 */
bool VertexAttribute::operator==(const VertexAttribute & other) const {
	return pimpl == other.pimpl;
}

/**
 *
 * @return
 */
bool VertexAttribute::validate() const {
	if (pimpl->vertexBuffer != nullptr) {
		return pimpl->vertexBuffer->validate();
	}
	return true;
}
