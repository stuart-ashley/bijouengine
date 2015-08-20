#include "canvas.h"

#include "fbo.h"
#include "texture.h"
#include "textureManager.h"

using namespace render;

struct Canvas::impl {
	std::unique_ptr<Fbo> fbo;
	std::vector<std::unique_ptr<Fbo>> fbos;

	impl() :
			fbo(nullptr) {
	}
};

/**
 * constructor
 */
Canvas::Canvas() :
		pimpl(new impl()) {
}

/**
 * destructor
 */
Canvas::~Canvas() {
	// screen target
	Fbo::disable();
}

/**
 *
 * @param texture
 * @return
 */
bool Canvas::setTarget(const std::shared_ptr<Texture> & texture) {
	if (pimpl->fbo != nullptr) {
		pimpl->fbo = nullptr;
	}

	if (texture == TextureManager::getInstance().getScreen()) {
		// screen target
		Fbo::disable();
		return true;
	}

	if (texture->validate() == false) {
		return false;
	}

	for (const auto & fbo : pimpl->fbos) {
		if (fbo->update(texture)) {
			return true;
		}
	}

	pimpl->fbos.emplace_back(new Fbo(texture));
	return true;
}

/**
 *
 * @param textures
 * @return
 */
bool Canvas::setTarget(const std::vector<std::shared_ptr<Texture>> & textures) {
	if (textures.size() == 1) {
		return setTarget(textures.at(0));
	}

	bool allTexturesValid = true;
	for (const auto & texture : textures) {
		allTexturesValid = allTexturesValid && texture->validate();
	}
	if (allTexturesValid == false) {
		return false;
	}

	if (pimpl->fbo == nullptr) {
		// new fbo
		pimpl->fbo = std::unique_ptr<Fbo>(new Fbo(textures));
		return true;
	}

	if (pimpl->fbo->update(textures) == false) {
		// can't use existing fbo
		pimpl->fbo = std::unique_ptr<Fbo>(new Fbo(textures));
	}
	return true;
}

/**
 *
 * @param textures
 * @param layer
 * @return
 */
bool Canvas::setTarget(const std::vector<std::shared_ptr<Texture>> & textures,
		int layer) {
	bool allTexturesValid = true;
	for (const auto & texture : textures) {
		allTexturesValid = allTexturesValid && texture->validate();
	}
	if (allTexturesValid == false) {
		return false;
	}

	if (pimpl->fbo == nullptr) {
		// new fbo
		pimpl->fbo = std::unique_ptr<Fbo>(new Fbo(textures, layer));
		return true;
	}

	if (pimpl->fbo->update(textures) == false) {
		// can't use existing fbo
		pimpl->fbo = std::unique_ptr<Fbo>(new Fbo(textures, layer));
	}
	return true;
}
