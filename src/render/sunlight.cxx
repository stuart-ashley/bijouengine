#include "sunlight.h"

#include "shaderManager.h"
#include "shaderTag.h"
#include "shadowView.h"
#include "texture.h"
#include "textureManager.h"
#include "uniform.h"
#include "uniformArray.h"

#include "../core/aspect.h"
#include "../core/boundingBox.h"
#include "../core/config.h"
#include "../core/orthographicCamera.h"
#include "../core/perspectiveCamera.h"

#include <cmath>
#include <memory>

using namespace render;

namespace {
	auto readRadiusUID = Uniform::getUID("u_readRadius");

	ShaderTag createOrthoShadowTag() {
		auto shader = ShaderManager::getInstance().getShadowTag();
		shader.addFlag(ShaderFlag::valueOf("ORTHO"));
		return shader;
	}

	const ShaderTag & getOrthoShadowTag() {
		static ShaderTag orthoShadowTag = createOrthoShadowTag();
		return orthoShadowTag;
	}

	/**
	 * Create new  shadow
	 *
	 * @param aspect     camera aspect for which to create shadow aspect
	 * @param nearZ      camera nearest z to include in new aspect
	 * @param farZ       camera farthest z to include in new aspect
	 * @param distance   far z of new aspect (camera bounds to be added)
	 * @param postfix    postfix for shadow camera name
	 * @param modifiers  shadow view modifiers
	 *
	 * @return          new shadow aspect containing bounds of camera view
	 * 					volume between nearZ and farZ
	 */
	Shadow getShadow(const Sunlight & light, const Aspect & aspect, float nearZ,
			float farZ, float distance, const std::string & postfix,
			const View::ModifierSet & modifiers) {

		const auto & aspectCamera = aspect.getCamera();
		const auto & aspectRotTrans = aspect.getRotTrans();
		auto aspectRot = aspectRotTrans.getRotation();

		// truncate far z if possible
		farZ = std::min(farZ, aspectCamera->getFar());

		Vec3Array corners = aspectCamera->getCorners(nearZ, farZ);
		// fix for transform
		Mat4 inverse = aspect.getInverseTransform();
		corners = corners.transformed(inverse);
		// transform frustum corners into sunlight camera space ( only need
		// rotation )
		Quat rotation = light.getTransform().getRotation();
		Quat t2sRotation = rotation.conjugate() * aspectRot;
		corners = corners.rotated(rotation.conjugate() * aspectRot);
		// get bounds
		const auto & bounds = corners.getBounds();
		auto halfExtent = bounds.getExtents() * .5;
		float hx = static_cast<float>(halfExtent.getX());
		float hy = static_cast<float>(halfExtent.getY());
		float hz = static_cast<float>(halfExtent.getZ());

		// transform centre of bounds to world
		Vec3 centre = rotation.rotate(bounds.getCentre());
		Vec3 aspectTranslation = aspectRotTrans.getTranslation();
		centre = centre + aspectTranslation;

		// name
		auto name = light.getId() + "." + aspectCamera->getName() + postfix;

		// create orthographic camera
		auto camera = std::make_shared<OrthographicCamera>(name,
				-hx, hx, -hy, hy, 10.f, distance + hz);

		// t = centre - direction * distance
		Vec3 t;
		t.scaleAdd(-distance, light.getDirection(), centre);

		Transform rotTrans(t, rotation);

		// build shadow
		Aspect shadowAspect(camera, rotTrans);
		auto texture = TextureManager::getInstance().getDepthTexture(name, 1024,
				1024, "lequal");
		UniformArray uniforms;
		uniforms.add(
				Uniform(readRadiusUID,
						static_cast<float>(2 * halfExtent.getX() * (2 + 5)
								/ texture->getWidth())));
		auto shadowView = std::make_shared<ShadowView>(name, shadowAspect,
				aspect, texture, Rect(0, 1, 0, 1), uniforms,
				getOrthoShadowTag(), View::Cull::BACK, View::DepthCompare::LESS,
				modifiers);

		return Shadow(texture, shadowView, shadowAspect, aspect);
	}
}

Sunlight::Sunlight(int id, const AbstractSunlight & light,
		const Transform & transform) :
		id(id), light(light), transform(transform), direction(0.f, 0.f, -1.f) {
	transform.rotate(direction);
}

/*
 *
 */
std::vector<Shadow> Sunlight::createShadowsForAspect(const Aspect & aspect,
		const View::ModifierSet & modifiers) const {

	int maxShadows = 1;
	float maxFar = 80;
	View::ModifierSet shadowModifiers;
	auto shadowType = Config::getInstance().getString("sunShadow");
	if (shadowType == "CASCADE_PCF_SHADOWS"
			|| shadowType == "CASCADE_SHADOWS") {
		if (modifiers.test(View::Modifier::LORES)) {
			shadowModifiers.set(View::Modifier::LORES);
		} else if (modifiers.test(View::Modifier::SNAPSHOT) == false) {
			maxShadows = 3;
		}
	}

	const auto & aspectCamera = aspect.getCamera();

	std::vector<Shadow> shadows;

	if (light.hasExtent()) {
		// name
		auto name = light.getName() + "." + aspectCamera->getName();
		// create orthographic camera
		float x = light.getExtent().getX() / 2;
		float y = light.getExtent().getY() / 2;
		auto camera = std::make_shared<OrthographicCamera>(name, -x, x, -y, y,
				10.f, maxFar);
		Aspect shadowAspect(camera, transform);

		auto texture = TextureManager::getInstance().getDepthTexture(name, 1024,
				1024, "lequal");

		UniformArray uniforms;
		uniforms.add(
				Uniform(readRadiusUID,
						2 * x * (2 + 5)
								/ static_cast<float>(texture->getWidth())));

		auto shadowView = std::make_shared<ShadowView>(name, shadowAspect,
				aspect, texture, Rect(0, 1, 0, 1), uniforms,
				getOrthoShadowTag(), View::Cull::BACK, View::DepthCompare::LESS,
				shadowModifiers);

		shadows.emplace_back(texture, shadowView, shadowAspect, aspect);

		return shadows;
	}

	float nearZ = .1f;
	float farZ = static_cast<float>(maxFar / std::pow(2, maxShadows - 1));
	for (int i = 0; i < maxShadows; ++i) {
		// early out if camera volume covered
		if (aspectCamera->getFar() < nearZ) {
			break;
		}
		shadows.emplace_back(
				getShadow(*this, aspect, nearZ, farZ, 200,
						"." + std::to_string(i), shadowModifiers));
		// update near and far
		nearZ = farZ;
		farZ = 2.f * farZ;
	}
	return shadows;
}

