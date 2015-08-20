#include "lights.h"

#include "../core/aspect.h"
#include "../core/intersection.h"

#include "../render/abstractProjectedKaleidoscope.h"
#include "../render/abstractProjectedTexture.h"
#include "../render/irradianceVolume.h"
#include "../render/lighting.h"
#include "../render/pointlight.h"
#include "../render/projectedKaleidoscope.h"
#include "../render/projectedTexture.h"
#include "../render/spotlight.h"
#include "../render/sunlight.h"

#include <mutex>
#include <unordered_map>

using namespace render;

struct Lights::impl {
	std::mutex m_lock;

	int counter;

	std::vector<Spotlight> spots;
	std::vector<Sunlight> suns;
	std::vector<Pointlight> points;

	/** placed but unlit projected textures */
	std::vector<std::pair<AbstractProjectedTexture, Transform>> unlitProjectedTextures;
	/** placed but unlit projected kaleidoscopes */
	std::vector<std::pair<AbstractProjectedKaleidoscope, Transform>> abstractProjectedKaleidoscopes;

	std::vector<IrradianceVolume> irradianceVolumes;

	std::unordered_map<int, std::vector<ProjectedTexture>> projectedTexturesByLight;
	std::unordered_map<int, std::vector<ProjectedKaleidoscope>> projectedKaleidoscopesByLight;

	impl() :
		counter(0) {
	}

	/**
	 * Shadow maps for spots for aspect
	 */
	std::vector<std::pair<Spotlight, Shadow>> calcSpotsForAspect(
			const Aspect & aspect,
			std::vector<std::shared_ptr<View>> & additionalViewTasks) {
		std::vector<std::pair<Spotlight, Shadow>> spotsForAspect;
		for (const auto & spot : spots) {
			Shadow shadow = spot.createShadowForAspect(aspect);
			additionalViewTasks.emplace_back(shadow.getView());
			spotsForAspect.emplace_back(spot, shadow);
		}

		return spotsForAspect;
	}

	/**
	 * Shadow maps for suns for aspect
	 *
	 * FIXME the aspect must always have the same modifiers
	 */
	std::vector<std::pair<Sunlight, std::vector<Shadow>>>calcSunsForAspect(
			const Aspect & aspect, const View::ModifierSet & modifiers,
			std::vector<std::shared_ptr<View>> & additionalViewTasks) {
		std::vector<std::pair<Sunlight, std::vector<Shadow>>> sunsForAspect;
		for (Sunlight sun : suns) {
			std::vector<Shadow> shadows = sun.createShadowsForAspect(aspect,
					modifiers);
			for (const auto & shadow : shadows) {
				additionalViewTasks.emplace_back(shadow.getView());
			}
			sunsForAspect.emplace_back(sun, shadows);
		}

		return sunsForAspect;
	}
};

/**
 * construct empty set of lights
 */
Lights::Lights() :
		pimpl(new impl()) {
}

/**
 * destructor
 */
Lights::~Lights() {
}

/*
 *
 */
void Lights::addIrradianceVolume(const render::IrradianceVolume & volume) {
	pimpl->irradianceVolumes.emplace_back(volume);
}

/*
 *
 */
void Lights::addPointlight(const AbstractLight & abstractPointlight,
		const Transform & transform) {
	pimpl->points.emplace_back(abstractPointlight, transform.getTranslation());
}

/*
 *
 */
void Lights::addProjectedKaleidoscope(
		const render::AbstractProjectedKaleidoscope & abstractProjectedKaleidoscope,
		const Transform & transform) {
	Intersection intxn;
	pimpl->abstractProjectedKaleidoscopes.emplace_back(
			abstractProjectedKaleidoscope, transform);
	for (const auto & sun : pimpl->suns) {
		auto & projectedKaleidoscopes =
				pimpl->projectedKaleidoscopesByLight[sun.getId()];
		projectedKaleidoscopes.emplace_back(abstractProjectedKaleidoscope,
				transform, sun);
	}
	for (const auto & spot : pimpl->spots) {
		auto & projectedKaleidoscopes =
				pimpl->projectedKaleidoscopesByLight[spot.getId()];
		if (spot.getConvexHull().collide(
				abstractProjectedKaleidoscope.getConvexHull(),
				transform.to(spot.getTransform()), intxn)) {
			projectedKaleidoscopes.emplace_back(abstractProjectedKaleidoscope,
					transform, spot);
		}
	}
}

/*
 *
 */
void Lights::addProjectedTexture(
		const render::AbstractProjectedTexture & abstractProjectedTexture,
		const Transform & transform) {
	pimpl->unlitProjectedTextures.emplace_back(abstractProjectedTexture,
			transform);

	// project by sunlight
	for (const auto & sun : pimpl->suns) {
		auto & projectedTextures = pimpl->projectedTexturesByLight[sun.getId()];
		projectedTextures.emplace_back(abstractProjectedTexture, transform,
				sun);
	}

	// project by spotlight if intersection
	Intersection intxn;
	for (Spotlight spot : pimpl->spots) {
		auto & projectedTextures = pimpl->projectedTexturesByLight[spot.getId()];
		if (spot.getConvexHull().collide(
				abstractProjectedTexture.getConvexHull(),
				transform.to(spot.getTransform()), intxn)) {
			projectedTextures.emplace_back(abstractProjectedTexture, transform,
					spot);
		}
	}
}

/*
 *
 */
void Lights::addSpotlight(const AbstractSpotlight & abstractSpotlight,
		const Transform & transform) {
	int id = ++pimpl->counter;
	Intersection intxn;
	Spotlight spotlight(id, abstractSpotlight, transform);
	std::vector<ProjectedTexture> projectedTexturesForLight;
	for (const auto & entry : pimpl->unlitProjectedTextures) {
		if (spotlight.getConvexHull().collide(entry.first.getConvexHull(),
				entry.second.to(spotlight.getTransform()), intxn)) {
			projectedTexturesForLight.emplace_back(entry.first, entry.second,
					spotlight);
		}
	}
	std::vector<ProjectedKaleidoscope> projectedKaleidoscopesForLight;
	for (const auto & entry : pimpl->abstractProjectedKaleidoscopes) {
		if (spotlight.getConvexHull().collide(entry.first.getConvexHull(),
				entry.second.to(spotlight.getTransform()), intxn)) {
			projectedKaleidoscopesForLight.emplace_back(entry.first,
					entry.second, spotlight);
		}
	}
	pimpl->spots.emplace_back(spotlight);
	auto r1 = pimpl->projectedTexturesByLight.emplace(id,
			projectedTexturesForLight);
	assert(r1.second);
	auto r2 = pimpl->projectedKaleidoscopesByLight.emplace(id,
			projectedKaleidoscopesForLight);
	assert(r2.second);
}

/*
 *
 */
void Lights::addSunlight(const AbstractSunlight & abstractSunlight,
		const Transform & transform) {
	int id = ++pimpl->counter;
	Sunlight sunlight(id, abstractSunlight, transform);
	std::vector<ProjectedTexture> projectedTexturesForLight;
	for (const auto & entry : pimpl->unlitProjectedTextures) {
		projectedTexturesForLight.emplace_back(entry.first, entry.second,
				sunlight);
	}
	std::vector<ProjectedKaleidoscope> projectedKaleidoscopesForLight;
	for (const auto & entry : pimpl->abstractProjectedKaleidoscopes) {
		projectedKaleidoscopesForLight.emplace_back(entry.first, entry.second,
				sunlight);
	}
	pimpl->suns.emplace_back(sunlight);
	auto r1 = pimpl->projectedTexturesByLight.emplace(id,
			projectedTexturesForLight);
	assert(r1.second);
	auto r2 = pimpl->projectedKaleidoscopesByLight.emplace(id,
			projectedKaleidoscopesForLight);
	assert(r2.second);
}

/*
 *
 */
render::Lighting Lights::getLighting(
		const std::shared_ptr<render::View> & view) const {
	if (view->isType(View::Type::SHADOW)) {
		return Lighting::empty();
	}
	assert(view->isType(View::Type::REGULAR));

	std::vector<std::shared_ptr<View>> additionalViewTasks;
	auto sunsForAspect = pimpl->calcSunsForAspect(view->getAspect(),
			view->getModifiers(), additionalViewTasks);
	auto spotsForAspect = pimpl->calcSpotsForAspect(view->getAspect(),
			additionalViewTasks);
	return Lighting(sunsForAspect, spotsForAspect, pimpl->points,
			pimpl->projectedTexturesByLight,
			pimpl->projectedKaleidoscopesByLight,
			pimpl->irradianceVolumes, additionalViewTasks);
}

/*
 *
 */
std::vector<DebugGeometry> Lights::getProjectedKaleidoscopesDebug() const {
	std::vector<DebugGeometry> debug;
	for (const auto & entry : pimpl->projectedKaleidoscopesByLight) {
		for (const auto & pk : entry.second) {
			debug.emplace_back(pk.getTransform(), Color::red(),
					pk.getConvexHull().getTriangleList());
		}
	}
	return debug;
}

/*
 *
 */
std::vector<DebugGeometry> Lights::getProjectedTexturesDebug() const {
	std::vector<DebugGeometry> debug;
	for (const auto & entry : pimpl->projectedTexturesByLight) {
		for (const auto & projectedTexture : entry.second) {
			debug.emplace_back(projectedTexture.getTransform(), Color::red(),
					projectedTexture.getConvexHull().getTriangleList());
		}
	}
	return debug;
}
