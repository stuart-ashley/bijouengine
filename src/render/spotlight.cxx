#include "spotlight.h"

#include "shaderManager.h"
#include "shadowView.h"
#include "textureManager.h"
#include "uniformArray.h"
#include "view.h"

#include "../core/aspect.h"
#include "../core/indexArray.h"
#include "../core/normalArray.h"
#include "../core/perspectiveCamera.h"

#include <cmath>

using namespace render;

/*
 *
 */
Spotlight::Spotlight(int id, const AbstractSpotlight & light,
		const Transform & transform) :
		id(id), light(light), transform(transform) {
}

/*
 *
 */
Shadow Spotlight::createShadowForAspect(const Aspect & aspect) const {
	auto name = light.getName() + "." + aspect.getCamera()->getName();

	auto camera = std::make_shared<PerspectiveCamera>(name, light.getAngle(), 1.f,
			.01f, light.getDistance());
	Aspect shadowAspect(camera, transform);

	auto texture = TextureManager::getInstance().getDepthTexture(name, 1024,
			1024, "lequal");

	UniformArray uniforms;
	auto shadowView = std::make_shared<ShadowView>(name, shadowAspect, aspect,
			texture, Rect(0, 1, 0, 1), uniforms,
			ShaderManager::getInstance().getShadowTag(), View::Cull::BACK,
			View::DepthCompare::LESS, View::ModifierSet());

	return Shadow(texture, shadowView, shadowAspect, aspect);
}

/**
 * get sine of half angle
 *
 * @return  sine of half angle
 */
float Spotlight::getSineHalfAngle() const {
	return std::sin(light.getHalfAngleRadians());
}
