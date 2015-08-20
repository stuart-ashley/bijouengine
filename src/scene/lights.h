#pragma once

#include "../core/debugGeometry.h"

#include "../render/lighting.h"

#include <array>
#include <memory>
#include <vector>

class AbstractLight;
class AbstractSpotlight;
class AbstractSunlight;
class Transform;

namespace render {
	class AbstractProjectedKaleidoscope;
	class AbstractProjectedTexture;
	class IrradianceVolume;
	class View;
}

class Lights {
public:

	/**
	 * construct empty set of lights
	 */
	Lights();

	/**
	 * destructor
	 */
	~Lights();

	void addIrradianceVolume(const render::IrradianceVolume & volume);

	void addPointlight(const AbstractLight & abstractPointlight,
			const Transform & transform);

	void addProjectedKaleidoscope(
			const render::AbstractProjectedKaleidoscope & abstractProjectedKaleidoscope,
			const Transform & transform);

	void addProjectedTexture(
			const render::AbstractProjectedTexture & abstractProjectedTexture,
			const Transform & transform);

	void addSpotlight(const AbstractSpotlight & abstractSpotlight,
			const Transform & transform);

	void addSunlight(const AbstractSunlight & abstractSunlight,
			const Transform & transform);

	render::Lighting getLighting(
			const std::shared_ptr<render::View> & view) const;

	std::vector<DebugGeometry> getProjectedKaleidoscopesDebug() const;

	std::vector<DebugGeometry> getProjectedTexturesDebug() const;

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
