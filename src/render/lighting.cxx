#include "lighting.h"

#include "irradianceVolume.h"
#include "pointlight.h"
#include "projectedKaleidoscope.h"
#include "projectedTexture.h"
#include "shadow.h"
#include "spotlight.h"
#include "sunlight.h"

#include "../core/boundingBox.h"
#include "../core/intersection.h"

#include <algorithm>

using namespace render;

struct Lighting::impl {
	std::vector<std::pair<Sunlight, ShadowList>> suns;
	std::vector<std::pair<Spotlight, Shadow>> spots;
	std::vector<Pointlight> points;
	std::unordered_map<int, std::vector<ProjectedTexture>> projectedTextures;
	std::unordered_map<int, std::vector<ProjectedKaleidoscope>> projectedKaleidoscopes;
	std::vector<IrradianceVolume> irradianceVolumes;

	std::vector<std::shared_ptr<View>> additionalViewTasks;

	impl() {
	}

	/**
	 *
	 * @param suns
	 * @param spots
	 * @param points
	 * @param projectedTextures
	 * @param projectedKaleidoscopes
	 * @param irradianceVolumes
	 * @param additionalViewTasks
	 */
	impl(const std::vector<std::pair<Sunlight, ShadowList>> & suns,
			const std::vector<std::pair<Spotlight, Shadow>> & spots,
			const std::vector<Pointlight> & points,
			const std::unordered_map<int, std::vector<ProjectedTexture>> & projectedTextures,
			const std::unordered_map<int, std::vector<ProjectedKaleidoscope>> & projectedKaleidoscopes,
			const std::vector<IrradianceVolume> & irradianceVolumes,
			const std::vector<std::shared_ptr<View>> & additionalViewTasks) :
					suns(suns),
					spots(spots),
					points(points),
					projectedTextures(projectedTextures),
					projectedKaleidoscopes(projectedKaleidoscopes),
					irradianceVolumes(irradianceVolumes),
					additionalViewTasks(additionalViewTasks) {
	}

	/**
	 *
	 * @param other
	 */
	impl(const impl & other) :
					suns(other.suns),
					spots(other.spots),
					points(other.points),
					projectedTextures(other.projectedTextures),
					projectedKaleidoscopes(other.projectedKaleidoscopes),
					irradianceVolumes(other.irradianceVolumes),
					additionalViewTasks(other.additionalViewTasks) {
	}

	/**
	 *
	 */
	void clipSpots(const BoundingBox & bounds, const Transform & transform) {
		auto end =
				std::remove_if(spots.begin(), spots.end(),
						[bounds, transform](const std::pair<Spotlight, Shadow> & entry) {
							Intersection intxn;
							Spotlight spot = entry.first;
							return spot.getConvexHull().collide(bounds,
									transform.to(spot.getTransform()), intxn);
						});
		spots.erase(end, spots.end());
	}

	/**
	 *
	 */
	void clipProjectedTextures(const BoundingBox & bounds,
			const Transform & transform) {
		for (auto & entry : projectedTextures) {
			auto & projectedTexturesForLight = entry.second;

			auto end =
					std::remove_if(projectedTexturesForLight.begin(),
							projectedTexturesForLight.end(),
							[bounds, transform](const ProjectedTexture & projectedTexture) {
								Intersection intxn;
								return projectedTexture.getConvexHull().collide(bounds,
										transform.to(projectedTexture.getTransform()), intxn);
							});
			projectedTexturesForLight.erase(end,
					projectedTexturesForLight.end());
		}
	}

	/**
	 *
	 */
	void clipProjectedKaleidoscopes(const BoundingBox & bounds,
			const Transform & transform) {
		for (auto & entry : projectedKaleidoscopes) {
			auto & projectedKaleidoscopesForLight = entry.second;

			auto end =
					std::remove_if(projectedKaleidoscopesForLight.begin(),
							projectedKaleidoscopesForLight.end(),
							[bounds, transform](const ProjectedKaleidoscope & projectedKaleidoscope) {
								Intersection intxn;
								return projectedKaleidoscope.getConvexHull().collide(bounds,
										transform.to(projectedKaleidoscope.getTransform()), intxn);
							});
			projectedKaleidoscopesForLight.erase(end,
					projectedKaleidoscopesForLight.end());
		}
	}
};

/**
 *
 */
Lighting::Lighting() :
		pimpl(new impl()) {
}

/**
 *
 * @param suns
 * @param spots
 * @param points
 * @param projectedTextures
 * @param projectedKaleidoscopes
 * @param irradianceVolumes
 * @param additionalViewTasks
 */
Lighting::Lighting(const std::vector<std::pair<Sunlight, ShadowList>> & suns,
		const std::vector<std::pair<Spotlight, Shadow>> & spots,
		const std::vector<Pointlight> & points,
		const std::unordered_map<int, std::vector<ProjectedTexture>> & projectedTextures,
		const std::unordered_map<int, std::vector<ProjectedKaleidoscope>> & projectedKaleidoscopes,
		const std::vector<IrradianceVolume> & irradianceVolumes,
		const std::vector<std::shared_ptr<View>> & additionalViewTasks) :
				pimpl(
						new impl(suns, spots, points, projectedTextures,
								projectedKaleidoscopes,
								irradianceVolumes, additionalViewTasks)) {

}

/**
 *
 * @param other
 */
Lighting::Lighting(const Lighting & other) :
		pimpl(new impl(*other.pimpl)) {

}

/**
 * destructor
 */
Lighting::~Lighting() {
}

/**
 *
 * @param bounds
 * @param transform
 */
void Lighting::clipLights(const BoundingBox & bounds,
		const Transform & transform) {
	pimpl->clipSpots(bounds, transform);
	pimpl->clipProjectedTextures(bounds, transform);
	pimpl->clipProjectedKaleidoscopes(bounds, transform);

	// clip irradiance volumes
	auto end = std::remove_if(pimpl->irradianceVolumes.begin(),
			pimpl->irradianceVolumes.end(),
			[bounds, transform](const IrradianceVolume & vol) {
				auto box = bounds.transformed(transform.to(vol.getTransform()));
				return vol.getVolume().getBounds().intersects(box);
			});
	pimpl->irradianceVolumes.erase(end, pimpl->irradianceVolumes.end());

	// maximise volume overlap
	auto best = pimpl->irradianceVolumes.begin();
	double maxOverlap = 0;
	for (auto itr = pimpl->irradianceVolumes.begin();
			itr != pimpl->irradianceVolumes.end(); ++itr) {
		auto box = bounds.transformed(transform.to(itr->getTransform()));

		Vec3 sumExtents = itr->getVolume().getBounds().getExtents()
				+ box.getExtents();

		BoundingBox boxUnion = itr->getVolume().getBounds() + box;
		Vec3 unionExtents = boxUnion.getExtents();

		Vec3 d = sumExtents - unionExtents;

		double minAxisOverlap = std::min(d.getX(),
				std::min(d.getY(), d.getZ()));

		if (minAxisOverlap > maxOverlap) {
			best = itr;
			maxOverlap = minAxisOverlap;
		}
	}
	if (best != pimpl->irradianceVolumes.begin()) {
		std::iter_swap(pimpl->irradianceVolumes.begin(), best);
	}
}

/**
 * get view tasks required by view
 */
const std::vector<std::shared_ptr<View>> & Lighting::getAdditionalViewTasks() const {
	return pimpl->additionalViewTasks;
}

/**
 * Get best irradiance volume
 */
const std::vector<IrradianceVolume> & Lighting::getIrradianceVolumes() const {
	return pimpl->irradianceVolumes;
}

/**
 *
 * @return
 */
const std::vector<Pointlight> & Lighting::getPointlights() const {
	return pimpl->points;
}

/**
 *
 * @param sunlight
 * @return
 */
const std::vector<ProjectedKaleidoscope> & Lighting::getProjectedKaleidoscopesForLight(
		int lightid) const {
	return pimpl->projectedKaleidoscopes.at(lightid);
}

/**
 *
 * @param sunlight
 * @return
 */
const std::vector<ProjectedTexture> & Lighting::getProjectedTexturesForLight(
		int lightid) const {
	return pimpl->projectedTextures.at(lightid);
}

/**
 *
 * @return
 */
const std::vector<std::pair<Spotlight, Shadow>> & Lighting::getSpotlights() const {
	return pimpl->spots;
}

/**
 *
 * @return
 */
const std::vector<std::pair<Sunlight, ShadowList>> & Lighting::getSunlights() const {
	return pimpl->suns;
}

STATIC const Lighting & Lighting::empty() {
	static Lighting empty;
	return empty;
}
