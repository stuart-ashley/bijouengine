#include "shadow.h"

#include "../core/aspect.h"

using namespace render;

namespace {
	/*
	 *
	 */
	Mat4 calcTextureMatrix(Aspect viewAspect, Aspect lightAspect) {
		// light camera
		const auto & camera = lightAspect.getCamera();

		// view to light matrix
		Mat4 v2l =
				viewAspect.getRotTrans().to(lightAspect.getRotTrans()).asMat4();

		// row major, texture matrix
		Mat4 m(.5f, 0.f, 0.f, .5f, 0.f, .5f, 0.f, .5f, 0.f, 0.f, .5f, .5f, 0.f,
				0.f, 0.f, 1.f);
		// light to texture
		Mat4 proj = camera->getProjectionMatrix();
		m.mul(proj);
		// fix for linear z buffer
		float near = camera->getNear();
		float far = camera->getFar();
		m = Mat4(m.get(0, 0), m.get(0, 1), m.get(0, 2), m.get(0, 3),
				m.get(1, 0), m.get(1, 1), m.get(1, 2), m.get(1, 3), m.get(2, 0),
				m.get(2, 1), 1 / (near - far), near / (near - far), m.get(3, 0),
				m.get(3, 1), m.get(3, 2), m.get(3, 3));
		// view to texture
		m.mul(v2l);

		return m;
	}
}

struct Shadow::impl {
	std::shared_ptr<Texture> texture;
	std::shared_ptr<View> view;
	Aspect aspect;
	Mat4 textureMatrix;
	Mat3 viewToLight;

	/*
	 *
	 */
	impl(const std::shared_ptr<Texture> & texture,
			const std::shared_ptr<View> & view, const Aspect & shadowAspect,
			const Aspect & viewerAspect) :
					texture(texture),
					view(view),
					aspect(shadowAspect),
					textureMatrix(
							calcTextureMatrix(viewerAspect, shadowAspect)),
					viewToLight(
							viewerAspect.getRotTrans().to(
									shadowAspect.getRotTrans()).getRotationMatrix()) {
	}
};

/*
 *
 */
Shadow::Shadow(const std::shared_ptr<Texture> & texture,
		const std::shared_ptr<View> & view, const Aspect & shadowAspect,
		const Aspect & viewerAspect) :
		pimpl(new impl(texture, view, shadowAspect, viewerAspect)) {
}

/**
 * get shadow texture
 *
 * @return  shadow texture
 */
const std::shared_ptr<Texture> & Shadow::getTexture() const {
	return pimpl->texture;
}

/**
 * get texture matrix
 *
 * @return  texture matrix
 */
const Mat4 & Shadow::getTextureMatrix() const {
	return pimpl->textureMatrix;
}

/**
 * get view for shadow
 *
 * @return  view for shadow
 */
const std::shared_ptr<View> & Shadow::getView() const {
	return pimpl->view;
}

/**
 * get view to light matrix
 *
 * @return  view to light matrix
 */
const Mat3 & Shadow::getViewToLight() const {
	return pimpl->viewToLight;
}
