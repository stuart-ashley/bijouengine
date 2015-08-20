#include "indexedTriangles.h"

#include "glDebug.h"

#include <cassert>

#include <GL/glew.h>

using namespace render;

struct IndexedTriangles::impl {
	IndexArray m_indices;
	unsigned m_vbo;

	impl(const IndexArray & indices) :
			m_indices(indices), m_vbo(0) {
	}
};

IndexedTriangles::IndexedTriangles(const IndexArray & indices) :
		pimpl(new impl(indices)) {
}

/**
 *
 */
void IndexedTriangles::drawElements() {
	assert(pimpl->m_indices.size() != 0);

	if (pimpl->m_vbo == 0) {
		glGenBuffers(1, &pimpl->m_vbo);
		assert(pimpl->m_vbo != 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pimpl->m_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint16_t) * pimpl->m_indices.size(),
				pimpl->m_indices.getBuffer(), GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pimpl->m_vbo);
	}

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pimpl->m_indices.size()),
			GL_UNSIGNED_SHORT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GlDebug::printOpenGLError();
}

/**
 *
 */
void IndexedTriangles::drawInstances(size_t nInstances) {
	assert(pimpl->m_indices.size() != 0);

	if (pimpl->m_vbo == 0) {
		glGenBuffers(1, &pimpl->m_vbo);
		assert(pimpl->m_vbo != 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pimpl->m_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				sizeof(uint16_t) * pimpl->m_indices.size(),
				pimpl->m_indices.getBuffer(), GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pimpl->m_vbo);
	}

	GlDebug::printOpenGLError();
	if (nInstances == 1) {
		glDrawElements(GL_TRIANGLES,
				static_cast<GLsizei>(pimpl->m_indices.size()), GL_UNSIGNED_SHORT,
				0);
	} else {
		glDrawElementsInstanced(GL_TRIANGLES,
				static_cast<GLsizei>(pimpl->m_indices.size()), GL_UNSIGNED_SHORT,
				0, static_cast<GLsizei>(nInstances));
	}
	GlDebug::printOpenGLError();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	GlDebug::printOpenGLError();
}

/**
 *
 * @return
 */
size_t IndexedTriangles::numTriangles() const {
	return pimpl->m_indices.size() / 3;
}

/**
 *
 * @param other
 * @return
 */
bool IndexedTriangles::operator==(const IndexedTriangles & other) const {
	return pimpl->m_indices == other.pimpl->m_indices;
}

/**
 *
 * @return
 */
bool IndexedTriangles::validate() const {
	return pimpl->m_indices.validate();
}
