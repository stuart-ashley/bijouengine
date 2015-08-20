#include "shaderFlag.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace render;

const std::vector<std::string> & ShaderFlag::values() {
	/**
	 * list of shader flags
	 * \showinitializer
	 */
	static std::vector<std::string> flags = {
	/** \verbatim DEPTH_RENDER                     shadow rendering  \endverbatim */
	"DEPTH_RENDER",
	/** \verbatim COLOR                            colour  \endverbatim */
	"COLOR",
	/** \verbatim COLORMAP                         colour map  \endverbatim */
	"COLORMAP",
	/** \verbatim SPECULAR                         specular  \endverbatim */
	"SPECULAR",
	/** \verbatim SPECULARMAP                      specular map  \endverbatim */
	"SPECULARMAP",
	/** \verbatim NORMAL                           normal  \endverbatim */
	"NORMAL",
	/** \verbatim NORMALMAP                        normal map  \endverbatim */
	"NORMALMAP",
	/** \verbatim POSITION                         position  \endverbatim */
	"POSITION",
	/** \verbatim LAMBERT                          lambert  \endverbatim */
	"LAMBERT",
	/** \verbatim PHONG                            phong  \endverbatim */
	"PHONG",
	/** \verbatim CASCADE_PCF_SHADOWS              cascade percentage closer filtered shadow maps  \endverbatim */
	"CASCADE_PCF_SHADOWS",
	/** \verbatim ENVMAP                           environment map  \endverbatim */
	"ENVMAP",
	/** \verbatim IGNORE_ENVMAP                    environment map  \endverbatim */
	"IGNORE_ENVMAP",
	/** \verbatim MIRROR                           mirror map  \endverbatim */
	"MIRROR",
	/** \verbatim REFRACTION                       refraction  \endverbatim */
	"REFRACTION",
	/** \verbatim SPOTLIGHT0                       first spotlight  \endverbatim */
	"SPOTLIGHT0",
	/** \verbatim SPOTLIGHT1                       second spotlight  \endverbatim */
	"SPOTLIGHT1",
	/** \verbatim SPOTLIGHT2                       third spotlight  \endverbatim */
	"SPOTLIGHT2",
	/** \verbatim SUNLIGHT                         sunlight  \endverbatim */
	"SUNLIGHT",
	/** \verbatim CASCADE1                         1 cascade shadow  \endverbatim */
	"CASCADE1",
	/** \verbatim CASCADE2                         2 cascade shadows  \endverbatim */
	"CASCADE2",
	/** \verbatim CASCADE3                         3 cascade shadows  \endverbatim */
	"CASCADE3",
	/** \verbatim ALPHAMAP                         alpha  \endverbatim */
	"ALPHAMAP",
	/** \verbatim UNLIT                            unlit  \endverbatim */
	"UNLIT",
	/** \verbatim SUNLIGHT_PROJECTED_TEXTURE0      first projected texture for sunlight  \endverbatim */
	"SUNLIGHT_PROJECTED_TEXTURE0",
	/** \verbatim SUNLIGHT_PROJECTED_TEXTURE1      second projected texture for sunlight  \endverbatim */
	"SUNLIGHT_PROJECTED_TEXTURE1",
	/** \verbatim SUNLIGHT_PROJECTED_TEXTURE2      third projected texture for sunlight  \endverbatim */
	"SUNLIGHT_PROJECTED_TEXTURE2",
	/** \verbatim SPOTLIGHT_PROJECTED_TEXTURE0     first projected texture for spotlight  \endverbatim */
	"SPOTLIGHT_PROJECTED_TEXTURE0",
	/** \verbatim SPOTLIGHT_PROJECTED_TEXTURE1     second projected texture for spotlight  \endverbatim */
	"SPOTLIGHT_PROJECTED_TEXTURE1",
	/** \verbatim SPOTLIGHT_PROJECTED_TEXTURE2     third projected texture for spotlight  \endverbatim */
	"SPOTLIGHT_PROJECTED_TEXTURE2",
	/** \verbatim SUNLIGHT_PROJECTED_KALEIDOSCOPE0 first projected kaleidoscope for sunlight  \endverbatim */
	"SUNLIGHT_PROJECTED_KALEIDOSCOPE0",
	/** \verbatim CLIP_PLANE0                      first clip plane  \endverbatim */
	"CLIP_PLANE0",
	/** \verbatim CLIP_PLANE1                      second clip plane  \endverbatim */
	"CLIP_PLANE1",
	/** \verbatim CLIP_PLANE2                      third clip plane  \endverbatim */
	"CLIP_PLANE2",
	/** \verbatim CLIP_PLANE3                      forth clip plane  \endverbatim */
	"CLIP_PLANE3",
	/** \verbatim SHADOW                           receives shadow  \endverbatim */
	"SHADOW",
	/** \verbatim CASCADE_SHADOWS                  cascade shadow maps  \endverbatim */
	"CASCADE_SHADOWS",
	/** \verbatim CASCADE_REGIONS                  cascade regions  \endverbatim */
	"CASCADE_REGIONS",
	/** \verbatim SSAO                             ssao  \endverbatim */
	"SSAO",
	/** \verbatim LIGHTMAP                         light map  \endverbatim */
	"LIGHTMAP",
	/** \verbatim DEFERRED                         deferred  \endverbatim */
	"DEFERRED",
	/** \verbatim SKINNING                         skinning  \endverbatim */
	"SKINNING",
	/** \verbatim IRRADIANCE_VOLUME                irradiance volume  \endverbatim */
	"IRRADIANCE_VOLUME",
	/** \verbatim DISTANCE_FADE                    fade by distance  \endverbatim */
	"DISTANCE_FADE",
	/** \verbatim VERTEX_TRANSFORM                 manipulate vertex position  \endverbatim */
	"VERTEX_TRANSFORM",
	/** \verbatim VOLUME                           rendering to volume  \endverbatim */
	"VOLUME",
	/** \verbatim IGNORE_IRRADIANCE                ignore irradiance  \endverbatim */
	"IGNORE_IRRADIANCE",
	/** \verbatim POINTLIGHT0                      first pointlight  \endverbatim */
	"POINTLIGHT0",
	/** \verbatim POINTLIGHT1                      second pointlight  \endverbatim */
	"POINTLIGHT1",
	/** \verbatim POINTLIGHT2                      third pointlight  \endverbatim */
	"POINTLIGHT2",
	/** \verbatim ORTHO                            orthographic  \endverbatim */
	"ORTHO",
	/** \verbatim LUMAMAP                          luma map  \endverbatim */
	"LUMAMAP",
	/** \verbatim OFFSETZ                          offset z  \endverbatim */
	"OFFSETZ",
	/** \verbatim SCALE_LIGHTING                   lighting scale  \endverbatim */
	"SCALE_LIGHTING",
	/** \verbatim INTENSITY_TEXTURE                intensity texture  \endverbatim */
	"INTENSITY_TEXTURE",
	/** \verbatim VERTEX_COLOR                     vertex color  \endverbatim */
	"VERTEX_COLOR", };

	assert(flags.size() == ShaderFlags().size());

	return flags;
}

size_t ShaderFlag::numValues() {
	return values().size();
}

const std::string & ShaderFlag::toString(size_t idx) {
	return values().at(idx);
}

size_t ShaderFlag::valueOf(const std::string & str) {
	const auto & vals = values();
	auto it = std::find(vals.begin(), vals.end(), str);
	assert(it != vals.end());
	return std::distance(vals.begin(), it);
}

size_t ShaderFlag::getSpotlightFlag(int i) {
	static auto spotlight0 = valueOf("SPOTLIGHT0");
	static auto spotlight1 = valueOf("SPOTLIGHT1");
	static auto spotlight2 = valueOf("SPOTLIGHT2");

	switch (i) {
	case 0:
		return spotlight0;
	case 1:
		return spotlight1;
	case 2:
		return spotlight2;
	default:
		std::cerr << "Spotlight index out of range" << std::endl;
		assert(false);
		return -1;
	}
}

size_t ShaderFlag::getSunlightProjectedTextureFlag(int i) {
	static auto sunlight_projected_texture0 = valueOf(
			"SUNLIGHT_PROJECTED_TEXTURE0");
	static auto sunlight_projected_texture1 = valueOf(
			"SUNLIGHT_PROJECTED_TEXTURE1");
	static auto sunlight_projected_texture2 = valueOf(
			"SUNLIGHT_PROJECTED_TEXTURE2");

	switch (i) {
	case 0:
		return sunlight_projected_texture0;
	case 1:
		return sunlight_projected_texture1;
	case 2:
		return sunlight_projected_texture2;
	default:
		std::cerr << "Projected texture index out of range" << std::endl;
		assert(false);
		return -1;
	}
}

size_t ShaderFlag::getSpotlightPorjectedTextureFlag(int i) {
	static auto spotlightProjectedTexture0 = valueOf(
			"SPOTLIGHT_PROJECTED_TEXTURE0");
	static auto spotlightProjectedTexture1 = valueOf(
			"SPOTLIGHT_PROJECTED_TEXTURE1");
	static auto spotlightProjectedTexture2 = valueOf(
			"SPOTLIGHT_PROJECTED_TEXTURE2");

	switch (i) {
	case 0:
		return spotlightProjectedTexture0;
	case 1:
		return spotlightProjectedTexture1;
	case 2:
		return spotlightProjectedTexture2;
	default:
		std::cerr << "Projected texture index out of range" << std::endl;
		assert(false);
		return -1;
	}
}

size_t ShaderFlag::getSunlightProjectedKaleidoscopeFlag(int i) {
	static auto sunlightProjectedKaleidoscope0 = valueOf(
			"SUNLIGHT_PROJECTED_KALEIDOSCOPE0");

	switch (i) {
	case 0:
		return sunlightProjectedKaleidoscope0;
	default:
		std::cerr << "Projected kaleidoscope index out of range" << std::endl;
		assert(false);
		return -1;
	}
}

size_t ShaderFlag::getClipPlaneFlag(size_t i) {
	static auto clipPlane0 = valueOf("CLIP_PLANE0");
	static auto clipPlane1 = valueOf("CLIP_PLANE1");
	static auto clipPlane2 = valueOf("CLIP_PLANE2");
	static auto clipPlane3 = valueOf("CLIP_PLANE3");

	switch (i) {
	case 0:
		return clipPlane0;
	case 1:
		return clipPlane1;
	case 2:
		return clipPlane2;
	case 3:
		return clipPlane3;
	default:
		std::cerr << "Clip plane index out of range" << std::endl;
		assert(false);
		return -1;
	}
}

size_t ShaderFlag::getPointlightFlag(int i) {
	static auto pointlight0 = valueOf("POINTLIGHT0");
	static auto pointlight1 = valueOf("POINTLIGHT1");
	static auto pointlight2 = valueOf("POINTLIGHT2");

	switch (i) {
	case 0:
		return pointlight0;
	case 1:
		return pointlight1;
	case 2:
		return pointlight2;
	default:
		std::cerr << "Pointlight index out of range" << std::endl;
		assert(false);
		return -1;
	}
}

/**
 * make shader flags from string list of flags
 *
 * @param list  string list of shader flags
 *
 * @return  shader flags
 */
ShaderFlags render::makeShaderFlags(std::vector<std::string> & list) {
	ShaderFlags flags;
	for (const auto & str : list) {
		flags[ShaderFlag::valueOf(str)] = true;
	}
	return flags;
}
