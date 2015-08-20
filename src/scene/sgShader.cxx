#include "sgShader.h"

#include "../core/aspect.h"
#include "../core/color.h"
#include "../core/config.h"
#include "../core/transform.h"
#include "../core/vec2.h"

#include "../render/irradianceVolume.h"
#include "../render/lighting.h"
#include "../render/pointlight.h"
#include "../render/projectedKaleidoscope.h"
#include "../render/projectedTexture.h"
#include "../render/renderState.h"
#include "../render/shaderManager.h"
#include "../render/spotlight.h"
#include "../render/sunlight.h"
#include "../render/texture.h"
#include "../render/uniform.h"
#include "../render/view.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <string>

using namespace render;

namespace {

	auto sunDirectionUID = Uniform::getUID("sunDirection");
	auto sunColorUID = Uniform::getUID("sunColor");
	auto sunViewToLightUID = Uniform::getUID("sunViewToLight");

	auto fadeNearUID = Uniform::getUID("u_fadeNear");
	auto fadeFarUID = Uniform::getUID("u_fadeFar");

	struct SpotUID {
		size_t point;
		size_t normal;
		size_t color;
		size_t sine;
		size_t distance;

		SpotUID(const std::string & prefix) :
						point(Uniform::getUID(prefix + ".point")),
						normal(Uniform::getUID(prefix + ".normal")),
						color(Uniform::getUID(prefix + ".color")),
						sine(Uniform::getUID(prefix + ".sine")),
						distance(Uniform::getUID(prefix + ".distance")) {
		}
	};

	SpotUID spotUIDs[] =
			{ SpotUID("spot0"), SpotUID("spot1"), SpotUID("spot2") };

	struct SpotShadowUID {
		size_t viewToLight;
		size_t texMat;

		SpotShadowUID(const std::string & prefix) :
						viewToLight(Uniform::getUID(prefix + ".viewToLight")),
						texMat(Uniform::getUID(prefix + ".texMat")) {
		}
	};

	SpotShadowUID spotShadowUIDs[] = { SpotShadowUID("spotShadow0"),
			SpotShadowUID("spotShadow1"), SpotShadowUID("spotShadow2") };

	size_t spotShadowMapUIDs[] =
			{ Uniform::getUID("spotShadowMap0"), Uniform::getUID(
					"spotShadowMap1"), Uniform::getUID("spotShadowMap2") };

	size_t pointlight_pointUIDs[] = { Uniform::getUID("u_pointlight0_point"),
			Uniform::getUID("u_pointlight1_point"), Uniform::getUID(
					"u_pointlight2_point") };

	size_t pointlight_colorUIDs[] = { Uniform::getUID("u_pointlight0_color"),
			Uniform::getUID("u_pointlight1_color"), Uniform::getUID(
					"u_pointlight2_color") };

	struct SunShadowUID {
		size_t texMat;
		size_t pixelWidth;

		SunShadowUID(const std::string & prefix) :
						texMat(Uniform::getUID(prefix + ".texMat")),
						pixelWidth(Uniform::getUID(prefix + ".pixelWidth")) {
		}
	};

	SunShadowUID sunShadowUIDs[] = { SunShadowUID("sunShadow0"), SunShadowUID(
			"sunShadow1"), SunShadowUID("sunShadow2") };

	size_t sunShadowMapUIDs[] = { Uniform::getUID("sunShadowMap0"),
			Uniform::getUID("sunShadowMap1"), Uniform::getUID("sunShadowMap2") };

	struct ProjectedTextureUID {
		size_t texMat;
		size_t alpha;
		size_t beta;

		ProjectedTextureUID(const std::string & prefix) :
						texMat(Uniform::getUID(prefix + ".texMat")),
						alpha(Uniform::getUID(prefix + ".alpha")),
						beta(Uniform::getUID(prefix + ".beta")) {
		}
	};

	ProjectedTextureUID spotlightProjectedTextureUIDs[] = { ProjectedTextureUID(
			"spotlightProjectedTexture0"), ProjectedTextureUID(
			"spotlightProjectedTexture1"), ProjectedTextureUID(
			"spotlightProjectedTexture2") };

	size_t spotlightProjectedTextureMapUIDs[] = { Uniform::getUID(
			"spotlightProjectedTextureMap0"), Uniform::getUID(
			"spotlightProjectedTextureMap1"), Uniform::getUID(
			"spotlightProjectedTextureMap2") };

	ProjectedTextureUID sunlightProjectedTextureUIDs[] = { ProjectedTextureUID(
			"sunlightProjectedTexture0"), ProjectedTextureUID(
			"sunlightProjectedTexture1"), ProjectedTextureUID(
			"sunlightProjectedTexture2") };

	size_t sunlightProjectedTextureMapUIDs[] = { Uniform::getUID(
			"sunlightProjectedTextureMap0"), Uniform::getUID(
			"sunlightProjectedTextureMap1"), Uniform::getUID(
			"sunlightProjectedTextureMap2") };

	struct ProjectedKaleidoscopeUID {
		size_t texMat;
		size_t segmentCentralAngle;
		size_t alpha;
		size_t beta;
		size_t uv0;
		size_t uv1;
		size_t uv2;

		ProjectedKaleidoscopeUID(const std::string & prefix) :
						texMat(Uniform::getUID(prefix + ".texMat")),
						segmentCentralAngle(
								Uniform::getUID(
										prefix + ".segmentCentralAngle")),
						alpha(Uniform::getUID(prefix + ".alpha")),
						beta(Uniform::getUID(prefix + ".beta")),
						uv0(Uniform::getUID(prefix + ".uv0")),
						uv1(Uniform::getUID(prefix + ".uv1")),
						uv2(Uniform::getUID(prefix + ".uv2")) {
		}
	};

	ProjectedKaleidoscopeUID sunlightProjectedKaleidoscopeUIDs[] = {
			ProjectedKaleidoscopeUID("sunlightProjectedKaleidoscope") };

	size_t sunlightProjectedKaleidoscopeMapUIDs[] = { Uniform::getUID(
			"sunlightProjectedKaleidoscopeMap") };

	auto irradianceVolumeTransformUID = Uniform::getUID(
			"u_irradianceVolumeTransform");
	auto irradianceVolumeExtentsUID = Uniform::getUID(
			"u_irradianceVolumeExtents");
	auto irradianceVolumeSizeUID = Uniform::getUID("u_irradianceVolumeSize");
	size_t irradianceVolumeTextureUIDs[] = { Uniform::getUID(
			"u_irradianceVolume0"), Uniform::getUID("u_irradianceVolume1"),
			Uniform::getUID("u_irradianceVolume2"), Uniform::getUID(
					"u_irradianceVolume3"), Uniform::getUID(
					"u_irradianceVolume4"), Uniform::getUID(
					"u_irradianceVolume5"), Uniform::getUID(
					"u_irradianceVolume6"), };

	auto viewToLumaUID = Uniform::getUID("u_viewToLuma");

	std::vector<BaseParameter> params = { Parameter<List>("flags",
			std::make_shared<List>()), Parameter<String>("vp",
			std::make_shared<String>("")), Parameter<String>("gp",
			std::make_shared<String>("")), Parameter<String>("fp",
			std::make_shared<String>("")) };

	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			const auto & flagsArg = std::static_pointer_cast<List>(
					args["flags"]);
			const auto & vpArg =
					std::static_pointer_cast<String>(args["vp"])->getValue();
			const auto & gpArg =
					std::static_pointer_cast<String>(args["gp"])->getValue();
			const auto & fpArg =
					std::static_pointer_cast<String>(args["fp"])->getValue();

			ShaderFlags flags;
			for (const auto & e : *flagsArg) {
				scriptExecutionAssertType<String>(e,
						"Require string flag argument");
				flags[ShaderFlag::valueOf(
						std::static_pointer_cast<String>(e)->getValue())] =
						true;
			}
			auto & shaderMan = ShaderManager::getInstance();
			std::shared_ptr<Program> vp =
					vpArg == "" ?
							nullptr : shaderMan.getProgram(currentDir, vpArg);
			std::shared_ptr<Program> gp =
					gpArg == "" ?
							nullptr : shaderMan.getProgram(currentDir, gpArg);
			std::shared_ptr<Program> fp =
					fpArg == "" ?
							nullptr : shaderMan.getProgram(currentDir, fpArg);
			stack.push(
					std::make_shared<SgShader>(ShaderTag(vp, gp, fp, flags)));
		}
	};
}

SgShader::SgShader(const render::ShaderTag & tag) :
		shaderTag(tag) {
}

/**
 * Process lights
 *
 * @param builder
 */
PRIVATE void SgShader::processLights(const render::ViewBuilder & vb) {
	auto & state = vb.getState();
	const auto & lighting = state.getLighting();
	const auto & aspect = vb.getView()->getAspect();

	ShaderFlags flags;
	int nSunlightProjectedTextures = 0;
	int nSpotlightProjectedTextures = 0;
	int nSunlightProjectedKaleidoscopes = 0;

	int numSpots = 0;
	for (auto spotItr = lighting.getSpotlights().begin(), end =
			lighting.getSpotlights().end(); spotItr != end && numSpots < 3;
			++spotItr, ++numSpots) {
		const auto & spot = spotItr->first;
		const auto & shadow = spotItr->second;

		// spot to view space
		auto s2v = spot.getTransform().to(aspect.getRotTrans());
		auto point = s2v.getTranslation();
		Normal normal(0.f, 0.f, -1.f);
		s2v.rotate(normal);
		// spotlight color
		const auto & color = spot.getColor();
		// uniforms
		state.addUniform(Uniform(spotUIDs[numSpots].point, point));
		state.addUniform(Uniform(spotUIDs[numSpots].normal, normal));
		state.addUniform(
				Uniform(spotUIDs[numSpots].color, color.getR(), color.getG(),
						color.getB()));
		state.addUniform(
				Uniform(spotUIDs[numSpots].distance, spot.getDistance()));
		state.addUniform(
				Uniform(spotUIDs[numSpots].sine, spot.getSineHalfAngle()));

		// shadow uniforms
		state.addUniform(
				Uniform(spotShadowUIDs[numSpots].viewToLight,
						shadow.getViewToLight()));
		state.addUniform(
				Uniform(spotShadowUIDs[numSpots].texMat,
						shadow.getTextureMatrix()));
		state.addUniform(
				Uniform(spotShadowMapUIDs[numSpots], shadow.getTexture()));
		flags[ShaderFlag::getSpotlightFlag(numSpots)] = true;

		// spotlight projected textures
		for (const auto & projectedTexture : lighting.getProjectedTexturesForLight(
				spot.getId())) {
			auto texmat = projectedTexture.getTexMat(aspect);
			state.addUniform(
					Uniform(
							spotlightProjectedTextureUIDs[nSpotlightProjectedTextures].texMat,
							texmat));
			state.addUniform(
					Uniform(
							spotlightProjectedTextureMapUIDs[nSpotlightProjectedTextures],
							projectedTexture.getTexture()));
			state.addUniform(
					Uniform(
							spotlightProjectedTextureUIDs[nSpotlightProjectedTextures].alpha,
							projectedTexture.getAlpha()));
			state.addUniform(
					Uniform(
							spotlightProjectedTextureUIDs[nSpotlightProjectedTextures].beta,
							projectedTexture.getBeta()));
			flags[ShaderFlag::getSpotlightPorjectedTextureFlag(
					nSpotlightProjectedTextures)] = true;
			nSpotlightProjectedTextures++;
			if (nSpotlightProjectedTextures >= 3) {
				break;
			}
		}
	}

	const auto & sunItr = lighting.getSunlights().begin();
	if (sunItr != lighting.getSunlights().end()) {
		const auto & sun = sunItr->first;
		const auto & shadows = sunItr->second;

		// sunlight direction in view space
		auto direction = sun.getDirection();
		aspect.getRotTrans().rotateInverse(direction);

		// uniforms
		state.addUniform(Uniform(sunDirectionUID, direction));
		const auto & sunColor = sun.getColor();
		state.addUniform(
				Uniform(sunColorUID, sunColor.getR(), sunColor.getG(),
						sunColor.getB()));
		state.addUniform(
				Uniform(sunViewToLightUID, shadows.at(0).getViewToLight()));

		flags[ShaderFlag::valueOf("SUNLIGHT")] = true;

		// shadow uniforms
		if (shaderTag.hasFlag(ShaderFlag::valueOf("SHADOW"))) {
			flags[vb.getShadowType()] = true;

			const auto nShadows = shadows.size();
			flags[ShaderFlag::valueOf("CASCADE" + std::to_string(nShadows))] =
					true;

			for (size_t j = 0; j < nShadows; ++j) {
				const auto & shadow = shadows[j];
				state.addUniform(
						Uniform(sunShadowUIDs[j].texMat,
								shadow.getTextureMatrix()));
				state.addUniform(
						Uniform(sunShadowMapUIDs[j], shadow.getTexture()));
				state.addUniform(
						Uniform(sunShadowUIDs[j].pixelWidth,
								static_cast<float>(shadow.getTexture()->getWidth())));
			}

		}

		// sunlight projected textures
		for (const auto & projectedTexture : lighting.getProjectedTexturesForLight(
				sun.getId())) {
			auto texmat = projectedTexture.getTexMat(aspect);
			state.addUniform(
					Uniform(
							sunlightProjectedTextureUIDs[nSunlightProjectedTextures].texMat,
							texmat));
			state.addUniform(
					Uniform(
							sunlightProjectedTextureMapUIDs[nSunlightProjectedTextures],
							projectedTexture.getTexture()));
			state.addUniform(
					Uniform(
							sunlightProjectedTextureUIDs[nSunlightProjectedTextures].alpha,
							projectedTexture.getAlpha()));
			state.addUniform(
					Uniform(
							sunlightProjectedTextureUIDs[nSunlightProjectedTextures].beta,
							projectedTexture.getBeta()));
			flags[ShaderFlag::getSunlightProjectedTextureFlag(
					nSunlightProjectedTextures)] = true;
			nSunlightProjectedTextures++;
			if (nSunlightProjectedTextures >= 3) {
				break;
			}
		}

		// sunlight projected kaleidoscope
		for (const auto & projectedKaleidoscope : lighting.getProjectedKaleidoscopesForLight(
				sun.getId())) {
			auto texmat = projectedKaleidoscope.getTexMat(aspect);
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].texMat,
							texmat));
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeMapUIDs[nSunlightProjectedKaleidoscopes],
							projectedKaleidoscope.getTexture()));
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].segmentCentralAngle,
							projectedKaleidoscope.getSegmentCentralAngleRadians()));
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].alpha,
							projectedKaleidoscope.getAlpha()));
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].beta,
							projectedKaleidoscope.getBeta()));
			const auto & uvs = projectedKaleidoscope.getUVs();
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].uv0,
							uvs[0]));
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].uv1,
							uvs[1]));
			state.addUniform(
					Uniform(
							sunlightProjectedKaleidoscopeUIDs[nSunlightProjectedKaleidoscopes].uv2,
							uvs[2]));
			flags[ShaderFlag::getSunlightProjectedKaleidoscopeFlag(
					nSunlightProjectedKaleidoscopes)] = true;
			nSunlightProjectedKaleidoscopes++;
			if (nSunlightProjectedKaleidoscopes >= 1) {
				break;
			}
		}
	}

	// point lights
	int numPoints = 0;
	for (auto pointItr = lighting.getPointlights().begin(), end =
			lighting.getPointlights().end(); pointItr != end && numPoints < 3;
			++pointItr, ++numPoints) {
		Vec3 p = pointItr->getPosition();
		// position to view space
		aspect.getRotTrans().inverseTransformPoint(p);

		state.addUniform(Uniform(pointlight_pointUIDs[numPoints], p));
		const auto & pointColor = pointItr->getColor();
		state.addUniform(
				Uniform(pointlight_colorUIDs[numPoints], pointColor.getR(),
						pointColor.getG(), pointColor.getB()));
		flags[ShaderFlag::getPointlightFlag(numPoints)] = true;
	}

	// irradiance volumes
	if (state.getBlend() == BlendFlag::NONE) {
		if (lighting.getIrradianceVolumes().size() > 0) {
			const auto & vol = lighting.getIrradianceVolumes().at(0);
			const auto & textures = vol.getVolume().getTextures();
			for (size_t i = 0; i < textures.size(); ++i) {
				state.addUniform(
						Uniform(irradianceVolumeTextureUIDs[i], textures[i]));
			}
			state.addUniform(
					Uniform(irradianceVolumeExtentsUID,
							vol.getVolume().getExtent()));
			state.addUniform(
					Uniform(irradianceVolumeSizeUID,
							static_cast<float>(vol.getVolume().getSizeX()),
							static_cast<float>(vol.getVolume().getSizeY()),
							static_cast<float>(vol.getVolume().getSizeZ())));
			auto m = aspect.getRotTrans().to(vol.getTransform()).asMat4();
			state.addUniform(Uniform(irradianceVolumeTransformUID, m));
			flags[ShaderFlag::valueOf("IRRADIANCE_VOLUME")] = true;
		}
	}

	// luminance map
	if (shaderTag.hasFlag(ShaderFlag::valueOf("LUMAMAP"))) {
		state.addUniform(
				Uniform(viewToLumaUID,
						aspect.getRotTrans().getRotationMatrix()));
	}

	// shader
	state.addShaderFlags(flags);
}

OVERRIDE void SgShader::visualize(render::ViewBuilder & vb) {
	if (shaderTag.validate() == false) {
		return;
	}
	auto & state = vb.getState();
	if (shaderTag.hasFlag(ShaderFlag::valueOf("SKINNING"))) {
		state.setupSkinning();
	}
	state.overlayShaderTag(shaderTag);
	if (vb.getView()->isType(View::Type::SHADOW) == false) {
		processLights(vb);
	}
	if (shaderTag.hasFlag(ShaderFlag::valueOf("DISTANCE_FADE"))
			&& Config::getInstance().getBoolean("overrideFade")) {
		float fadeNear = Config::getInstance().getFloat("fadeNear");
		float fadeFar = Config::getInstance().getFloat("fadeFar");

		state.addUniform(Uniform(fadeNearUID, fadeNear));
		state.addUniform(Uniform(fadeFarUID, fadeFar));
	}
}

/**
 * get script object factory for SgShader
 *
 * @param currentDir  current directory of caller
 *
 * @return            SgShader factory
 */
STATIC ScriptObjectPtr SgShader::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
