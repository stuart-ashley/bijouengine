#include "view.h"

#include "shaderTag.h"
#include "uniformArray.h"

#include "../core/aspect.h"
#include "../core/boundingBox.h"
#include "../core/intersection.h"
#include "../core/transform.h"

#include <cassert>

using namespace render;

struct View::impl {
	Aspect aspect;
	Aspect lodAspect;
	UniformArray uniforms;
	ShaderTag shaderTag;
	Cull cull;
	DepthCompare depthCompare;
	ModifierSet modifiers;

	impl(const Aspect & aspect, const Aspect & lodAspect,
			const UniformArray & uniforms, const ShaderTag & shader, Cull cull,
			DepthCompare depthCompare, const ModifierSet & modifiers) :
					aspect(aspect),
					lodAspect(lodAspect),
					uniforms(uniforms),
					shaderTag(shader),
					cull(cull),
					depthCompare(depthCompare),
					modifiers(modifiers) {
	}
};

View::View(const std::string & name, const Aspect & aspect,
		const Aspect & lodAspect,
		const std::vector<std::shared_ptr<Texture>> & textures,
		const Rect & rect, const UniformArray & uniforms,
		const ShaderTag & shader, Cull cull, DepthCompare depthCompare,
		const ModifierSet & modifiers, int level) :
				RenderTask(name, textures, rect, level),
				pimpl(
						new impl(aspect, lodAspect, uniforms, shader, cull,
								depthCompare, modifiers)) {
}

View::View(const std::string & name, const Aspect & aspect,
		const Aspect & lodAspect, const std::shared_ptr<Texture> & texture,
		const Rect & rect, const UniformArray & uniforms,
		const ShaderTag & shader, Cull cull, DepthCompare depthCompare,
		const ModifierSet & modifiers, int level) :
				RenderTask(name, texture, rect, level),
				pimpl(
						new impl(aspect, lodAspect, uniforms, shader, cull,
								depthCompare, modifiers)) {
}

/**
 * destructor
 */
View::~View() {
}

/**
 * get aspect for view
 *
 * @return  view aspect
 */
const Aspect & View::getAspect() const {
	return pimpl->aspect;
}

/**
 * get culling mode for view
 *
 * @return  view culling mode
 */
View::Cull View::getCull() const {
	return pimpl->cull;
}

/**
 * get depth compare mode for view
 *
 * @return  view depth compare mode
 */
View::DepthCompare View::getDepthCompare() const {
	return pimpl->depthCompare;
}

/**
 * get lod aspect for view
 *
 * @return  view lod aspect
 */
const Aspect & View::getLodAspect() const {
	return pimpl->lodAspect;
}

/**
 * get modifiers for view
 *
 * @return  view modifiers
 */
const View::ModifierSet & View::getModifiers() const {
	return pimpl->modifiers;
}

/**
 * get shader tag for view
 *
 * @return  shader tag
 */
const ShaderTag & View::getShaderTag() const {
	return pimpl->shaderTag;
}

/**
 * get uniforms for view
 *
 * @return  uniforms
 */
const UniformArray & View::getUniforms() const {
	return pimpl->uniforms;
}

/**
 * Check bounding box is visible by checking for intersection with camera
 * frustum
 *
 * @param box
 *            bounding box to test
 * @param boxToWorld
 *            transform to take box from local to world space
 * @return true if box intersects frustum, false otherwise
 */
bool View::isVisible(const BoundingBox & box, const Transform & boxToWorld) {
	//
	// first test box as sphere against hull. If centre is more than radius
	// from plane it is outside and can be dropped
	//

	// model to camera space
	auto m2c = boxToWorld.to(pimpl->aspect.getRotTrans());
	// box centre to camera space
	auto centre = box.getCentre();
	m2c.transformPoint(centre);
	// radius
	double radius = box.getRadius();
	// box centre inside or on hull flag
	bool inside = true;

	for (const auto & p : pimpl->aspect.getConvexHull().getPlanes()) {
		double d = p.distanceTo(centre);
		if (d > radius) {
			return false;
		}
		inside = inside && (d <= 0);
	}

	//
	// further testing if centre not inside, compare hull to box
	//
	if (inside == false) {
		Intersection intxn;
		if (pimpl->aspect.getConvexHull().collide(box, m2c, intxn) == false) {
			return false;
		}
	}

	//
	// clip plane test
	//
	Transform worldToBoxSpace = boxToWorld.inverse();
	for (const auto & plane : pimpl->aspect.getClipPlanes()) {
		// world space plane in model space
		auto p = plane;
		p.transform(worldToBoxSpace);
		if (p.isAbove(box)) {
			return false;
		}
	}

	return true;
}
