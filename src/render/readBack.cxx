#include "readBack.h"

#include "canvas.h"
#include "texture.h"

#include <cassert>

#include <GL/glew.h>

using namespace render;

struct ReadBack::impl {
	std::shared_ptr<Texture> texture;
	int x;
	int y;
	int width;
	int height;
	std::vector<uint8_t> bytes;
	std::vector<float> floats;

	impl(const std::shared_ptr<Texture> & texture, int x, int y, int width,
			int height) :
			texture(texture), x(x), y(y), width(width), height(height) {
	}
};

/**
 *
 * @param texture
 * @param x
 * @param y
 * @param width
 * @param height
 */
ReadBack::ReadBack(const std::shared_ptr<Texture> & texture, int x, int y,
		int width, int height) :
		pimpl(new impl(texture, x, y, width, height)) {
}

/**
 * destructor
 */
ReadBack::~ReadBack(){
}

/**
 *
 * @return
 */
const std::vector<uint8_t> & ReadBack::getBytes() const {
	return pimpl->bytes;
}

/**
 *
 */
void ReadBack::execute(Canvas & canvas) {
	if (canvas.setTarget(pimpl->texture) == false) {
		return;
	}
	switch (pimpl->texture->getInternalFormat()) {
	case GL_RGBA8:
		pimpl->bytes.reserve(pimpl->width * pimpl->height * 4);
		glReadPixels(pimpl->x, pimpl->y, pimpl->width, pimpl->height, GL_RGBA,
				GL_UNSIGNED_BYTE, pimpl->bytes.data());
		glFlush();
		break;
	case GL_RGBA16F:
		pimpl->floats.reserve(pimpl->width * pimpl->height * 4 * 4);
		glReadPixels(pimpl->x, pimpl->y, pimpl->width, pimpl->height, GL_RGBA,
				GL_FLOAT, pimpl->floats.data());
		glFlush();
		break;
	default:
		assert(false);
	}
}
