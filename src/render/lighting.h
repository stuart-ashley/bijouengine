#pragma once

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

class BoundingBox;
class Transform;

namespace render {
	class IrradianceVolume;
	class Pointlight;
	class ProjectedKaleidoscope;
	class ProjectedTexture;
	class Shadow;
	class Spotlight;
	class Sunlight;
	class View;

	typedef std::vector<Shadow> ShadowList;

	class Lighting {
	public:

		/**
		 *
		 */
		Lighting();

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
		Lighting(const std::vector<std::pair<Sunlight, ShadowList>> & suns,
				const std::vector<std::pair<Spotlight, Shadow>> & spots,
				const std::vector<Pointlight> & points,
				const std::unordered_map<int, std::vector<ProjectedTexture>> & projectedTextures,
				const std::unordered_map<int, std::vector<ProjectedKaleidoscope>> & projectedKaleidoscopes,
				const std::vector<IrradianceVolume> & irradianceVolumes,
				const std::vector<std::shared_ptr<View>> & additionalViewTasks);

		/**
		 *
		 * @param other
		 */
		Lighting(const Lighting & other);

		/**
		 * destructor
		 */
		~Lighting();

		/**
		 *
		 * @param bounds
		 * @param transform
		 */
		void clipLights(const BoundingBox & bounds,
				const Transform & transform);

		/**
		 * get view tasks required by view
		 */
		const std::vector<std::shared_ptr<View>> & getAdditionalViewTasks() const;

		/**
		 * Get best irradiance volume
		 */
		const std::vector<IrradianceVolume> & getIrradianceVolumes() const;

		/**
		 *
		 * @return
		 */
		const std::vector<Pointlight> & getPointlights() const;

		/**
		 *
		 * @param sunlight
		 * @return
		 */
		const std::vector<ProjectedKaleidoscope> & getProjectedKaleidoscopesForLight(
				int lightid) const;

		/**
		 *
		 * @param sunlight
		 * @return
		 */
		const std::vector<ProjectedTexture> & getProjectedTexturesForLight(
				int lightid) const;

		/**
		 *
		 * @return
		 */
		const std::vector<std::pair<Spotlight, Shadow>> & getSpotlights() const;

		/**
		 *
		 * @return
		 */
		const std::vector<std::pair<Sunlight, ShadowList>> & getSunlights() const;

		static const Lighting & empty();

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

