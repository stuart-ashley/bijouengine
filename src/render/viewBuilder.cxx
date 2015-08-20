#include "viewBuilder.h"

#include "indexedTriangles.h"
#include "lighting.h"
#include "regularView.h"
#include "renderState.h"
#include "shaderManager.h"
#include "textureManager.h"
#include "uniform.h"
#include "uniformArray.h"
#include "view.h"

#include "../core/aspect.h"
#include "../core/boundingBox.h"
#include "../core/config.h"
#include "../core/convexHull.h"
#include "../core/indexArray.h"
#include "../core/occlusionMap.h"
#include "../core/perspectiveCamera.h"
#include "../core/plane.h"

#include <cmath>
#include <stack>

using namespace render;

namespace {
	struct Mirror {
		Plane plane;
		Uniform uniform;

		Mirror(const Plane & plane, const Uniform & uniform) :
				plane(plane), uniform(uniform) {
		}

		bool matches(const Plane & p) const {
			if (std::abs(plane.getNormal().getX() - p.getNormal().getX())
					+ std::abs(plane.getNormal().getY() - p.getNormal().getY())
					+ std::abs(plane.getNormal().getZ() - p.getNormal().getZ())
					> .1f) {
				return false;
			}
			Vec3 v = p.getPoint() - plane.getPoint();
			double d = v.dot(plane.getNormal());
			if (d < -.1f || d > .1f) {
				return false;
			}
			return true;
		}
	};

	auto nearUID = Uniform::getUID("u_near");
	auto farUID = Uniform::getUID("u_far");
	auto tanHalfFovyUID = Uniform::getUID("u_tanHalfFovy");
	auto aspectRatioUID = Uniform::getUID("u_aspectRatio");
	auto destDimensionsUID = Uniform::getUID("u_destDimensions");

	auto ReflectionUID = Uniform::getUID("Reflection");
	auto RefractionUID = Uniform::getUID("Refraction");

	size_t calcShadowType() {
		return ShaderFlag::valueOf(Config::getInstance().getString("sunShadow"));
	}
}

struct ViewBuilder::impl {
	std::shared_ptr<View> currentView;
	std::stack<RenderState> stateStack;

	std::vector<std::shared_ptr<View>> additionalViewTasks;

	std::vector<Mirror> mirrors;
	std::vector<Mirror> refractions;

	OcclusionMap occlusionMap;

	size_t numPolygons;

	bool showCollisions;

	size_t shadowType;

	impl(const std::shared_ptr<View> & view,
			const std::vector<std::shared_ptr<View>> & additionalViewTasks) :
					currentView(view),
					additionalViewTasks(additionalViewTasks),
					occlusionMap(view->getAspect(), 32, 32),
					numPolygons(0),
					showCollisions(
							Config::getInstance().getBoolean("showCollisions")),
					shadowType(calcShadowType()) {
	}
};

/**
 * Constructor
 */
ViewBuilder::ViewBuilder(const std::shared_ptr<View> & view,
		const Lighting & lighting) :
		pimpl(new impl(view, lighting.getAdditionalViewTasks())) {

	const auto & camera = view->getAspect().getCamera();

	pimpl->stateStack.emplace(lighting, view->getShaderTag());
	// camera uniforms
	getState().addUniform(Uniform(nearUID, camera->getNear()));
	getState().addUniform(Uniform(farUID, camera->getFar()));
	if (typeid(*camera) == typeid(PerspectiveCamera)) {
		auto persCam = std::static_pointer_cast<PerspectiveCamera>(camera);
		getState().addUniform(
				Uniform(tanHalfFovyUID, persCam->getTanHalfFovy()));
	}
	getState().addUniform(Uniform(aspectRatioUID, camera->getAspectRatio()));
	// target size
	Rect rect = view->getViewport();
	getState().addUniform(
			Uniform(destDimensionsUID, rect.getWidth(), rect.getHeight()));
	// view uniforms
	getState().addUniforms(view->getUniforms());

	// set extra flags for clip planes
	const auto & clipPlanes = view->getAspect().getClipPlanes();
	ShaderFlags flags;
	if (clipPlanes.size() > 0) {
		for (size_t i = 0, n = clipPlanes.size(); i < n; ++i) {
			flags[ShaderFlag::getClipPlaneFlag(i)] = true;
		}
	}
	getState().addShaderFlags(flags);
}

/**
 * destructor
 */
ViewBuilder::~ViewBuilder() {
}

void ViewBuilder::addAlphaIndexedTriangles(const IndexedTriangles & indices,
		float z) {
	if (indices.numTriangles() > 0 && getState().validate()) {
		pimpl->currentView->addAlphaIndexedTriangles(getState(), indices, z);
	}
}

void ViewBuilder::addDebugGeometry(const std::vector<DebugGeometry> & debug) {
	pimpl->currentView->addDebugGeometry(debug);
}

void ViewBuilder::addEdgeIndexArray(const IndexArray & indices) {
	if (indices.size() > 0 && getState().validate()) {
		pimpl->currentView->addEdgeIndexArray(getState(), indices);
	}
}

void ViewBuilder::addIndexedTriangles(const IndexedTriangles & indices) {
	if (indices.numTriangles() > 0 && getState().validate()) {
		pimpl->currentView->addIndexedTriangles(getState(), indices);
	}
}

/**
 * Add occluder to view
 */
void ViewBuilder::addOccluders(const std::vector<ConvexHull> & occluders) {
	for (const auto & hull : occluders) {
		pimpl->occlusionMap.write(hull);
	}
}

void ViewBuilder::addPoint() {
	if (getState().validate()) {
		pimpl->currentView->addPoint(getState());
	}
}

void ViewBuilder::addSphere(float radius) {
	if (getState().validate()) {
		pimpl->currentView->addSphere(getState(), radius);
	}
}

/**
 * add extra view required by current view
 *
 * @param view  extra view
 */
void ViewBuilder::addView(const std::shared_ptr<View> & view) {
	pimpl->additionalViewTasks.emplace_back(view);
}

/**
 * add plane for volumetric lighting
 *
 * @param triangles  triangle indices for region to apply volumetric effect to
 */
void ViewBuilder::addVolumetric(const IndexedTriangles & triangles) {
	if (getState().validate()) {
		pimpl->currentView->addVolumetric(getState(), triangles);
	}
}
/**
 * Apply reflection texture uniform for plane, firstly try find existing, if
 * that fails create new
 *
 * @param worldPlane
 *            plane of reflection
 */
void ViewBuilder::applyMirror(const std::string & name,
		const Plane & worldPlane) {
	static auto mirrorFlag = ShaderFlag::valueOf("MIRROR");
	getState().addShaderFlag(mirrorFlag);

	const auto & aspect = pimpl->currentView->getAspect();
	for (const auto & mirror : pimpl->mirrors) {
		if (mirror.matches(worldPlane)) {
			getState().addUniform(mirror.uniform);
			return;
		}
	}

	// duplicate target camera but rename
	auto camera = aspect.getCamera()->clone(name);

	// duplicate aspect and mirror
	Aspect mirroredAspect(camera, aspect.getRotTrans());
	mirroredAspect.transform(aspect.getTransform());
	mirroredAspect.mirror(worldPlane);

	// add plane to clip planes (with small offset to avoid depth fighting)
	Vec3 p;
	p.scaleAdd(-.0001, worldPlane.getNormal(), worldPlane.getPoint());
	mirroredAspect.addClipPlane(Plane(p, worldPlane.getNormal()));

	// target texture
	auto r = pimpl->currentView->getViewport();
	auto tex = TextureManager::getInstance().getTexture(name,
			(int) r.getWidth(), (int) r.getHeight(), true);
	// texture uniform
	Uniform uniform(ReflectionUID, tex);
	getState().addUniform(uniform);
	// add view
	View::ModifierSet lores;
	lores[View::Modifier::LORES] = true;
	pimpl->additionalViewTasks.emplace_back(
			std::make_shared<RegularView>(name, mirroredAspect, tex,
					Rect(0, 1, 0, 1), UniformArray(),
					ShaderManager::getInstance().getUberTag(), View::Cull::BACK,
					View::DepthCompare::LESS, lores, 0));

	// add to cache
	pimpl->mirrors.emplace_back(worldPlane, uniform);
}

/**
 * Apply refraction texture uniform for plane, firstly try find existing, if
 * that fails create new
 *
 * @param worldPlane
 *            plane of refraction
 */
void ViewBuilder::applyRefraction(const std::string & name,
		const Plane & worldPlane) {
	static auto refractionFlag = ShaderFlag::valueOf("REFRACTION");
	getState().addShaderFlag(refractionFlag);

	const auto & aspect = pimpl->currentView->getAspect();
	for (const auto & mirror : pimpl->refractions) {
		if (mirror.matches(worldPlane)) {
			getState().addUniform(mirror.uniform);
			return;
		}
	}

	// duplicate target camera but rename
	auto camera = aspect.getCamera()->clone(name);

	// duplicate aspect
	Aspect refractionAspect(camera, aspect.getRotTrans());
	refractionAspect.transform(aspect.getTransform());

	// add clip plane (with small offset to avoid depth fighting)
	Vec3 p;
	p.scaleAdd(.0001, worldPlane.getNormal(), worldPlane.getPoint());
	refractionAspect.addClipPlane(Plane(p, worldPlane.getNormal()));

	// target texture
	auto r = pimpl->currentView->getViewport();
	auto tex = TextureManager::getInstance().getTexture(name,
			(int) r.getWidth(), (int) r.getHeight(), true);
	// texture uniform
	Uniform uniform(RefractionUID, tex);
	getState().addUniform(uniform);
	// add view
	pimpl->additionalViewTasks.emplace_back(
			std::make_shared<RegularView>(name, refractionAspect, tex,
					Rect(0, 1, 0, 1), UniformArray(),
					ShaderManager::getInstance().getUberTag(), View::Cull::BACK,
					View::DepthCompare::LESS, View::ModifierSet(), 0));

	// add to cache
	pimpl->refractions.emplace_back(worldPlane, uniform);
}

/**
 * get additional view tasks for view
 *
 * @return  views required for view
 */
const std::vector<std::shared_ptr<View>> & ViewBuilder::getAdditionalViewTasks() const {
	return pimpl->additionalViewTasks;
}

size_t ViewBuilder::getPolyCount() const {
	return pimpl->numPolygons;
}

/**
 * get type of shadows
 *
 * @return  type of shadows
 */
size_t ViewBuilder::getShadowType() const {
	return pimpl->shadowType;
}

RenderState & ViewBuilder::getState() const {
	return pimpl->stateStack.top();
}

const std::shared_ptr<View> & ViewBuilder::getView() const {
	return pimpl->currentView;
}

void ViewBuilder::incPolyCount(size_t n) {
	pimpl->numPolygons += n;
}

/**
 * Check bounding box is visible, by first checking for intersection with
 * camera frustum, and then testing against occlusion geometry
 *
 * @param box
 * @return
 */
bool ViewBuilder::isVisible(const BoundingBox & box) const {
	const auto transform = getState().getTransform();

	if (pimpl->currentView->isVisible(box, transform) == false) {
		return false;
	}

	//
	// occlusion test
	//
	if (pimpl->occlusionMap.isOccluded(transform, ConvexHull(box))) {
		return false;
	}

	return true;
}

/**
 * Pop state
 */
void ViewBuilder::popState() {
	pimpl->stateStack.pop();
}

/**
 * Push state
 */
void ViewBuilder::pushState() {
	pimpl->stateStack.emplace(pimpl->stateStack.top());
}

/**
 * Show collisions
 */
bool ViewBuilder::showCollisions() {
	return pimpl->showCollisions;
}
