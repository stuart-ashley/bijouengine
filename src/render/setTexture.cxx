#include "setTexture.h"

#include "texture.h"

#include <cassert>

#include <GL/glew.h>

using namespace render;

SetTexture::SetTexture(const std::string & name,
		const std::shared_ptr<Texture> & texture, const char * data) :
				RenderTask(name, texture, Rect(0, 1, 0, 1), 0),
				data(data),
				executed(false) {
}

/**
 * destructor
 */
SetTexture::~SetTexture() {
}

OVERRIDE void SetTexture::execute(Canvas & canvas) {
	if (executed) {
		return;
	}
	auto target = getTargetTextures().at(0);
	target->bind();
	switch (target->getType()) {
	case GL_TEXTURE_3D:
		switch (target->getInternalFormat()) {
		case GL_RGBA8:
			glTexImage3D(target->getType(), 0, target->getInternalFormat(),
					target->getWidth(), target->getHeight(), target->getDepth(),
					0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
		case GL_RGBA16F:
			glTexImage3D(target->getType(), 0, target->getInternalFormat(),
					target->getWidth(), target->getHeight(), target->getDepth(),
					0, GL_RGBA, GL_FLOAT, data);
			break;
		default:
			assert(false);
		}
		break;
	default:
		assert(false);
	}
	glFinish();

	executed = true;
}

OVERRIDE bool SetTexture::hasExecuted() const {
	return executed;
}
