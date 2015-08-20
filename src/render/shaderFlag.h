#pragma once

#include <bitset>
#include <string>
#include <vector>

namespace render {
	class ShaderFlag {
	public:

		static const std::vector<std::string> & values();

		static size_t numValues();

		static size_t valueOf(const std::string & str);

		static const std::string & toString(size_t idx);

		static size_t getSpotlightFlag(int i);

		static size_t getSunlightProjectedTextureFlag(int i);

		static size_t getSpotlightPorjectedTextureFlag(int i);

		static size_t getSunlightProjectedKaleidoscopeFlag(int i);
		static size_t getClipPlaneFlag(size_t i);

		static size_t getPointlightFlag(int i);

		static size_t getIrradianceFlag(int i);
	};

	typedef std::bitset<56> ShaderFlags;

	/**
	 * make shader flags from string list of flags
	 *
	 * @param list  string list of shader flags
	 *
	 * @return  shader flags
	 */
	ShaderFlags makeShaderFlags(std::vector<std::string> & list);
}

