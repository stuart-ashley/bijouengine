#pragma once

#include "texture.h"

#include <bitset>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace render {
	class TextureManager {
	public:
		enum Flag {
			RGBA8, RGBA16F, RGBA32F, FILTER, maxValue
		};
		typedef std::bitset<Flag::maxValue> FlagSet;

		/**
		 *
		 */
		TextureManager();

		void freshen(const std::unordered_set<int> & textures);

		/**
		 * get 3D texture
		 *
		 * @param name    texture name
		 * @param width   texture width
		 * @param height  texture height
		 * @param depth   texture depth
		 * @param flags   texture flags
		 *
		 * @return        3d texture
		 */
		std::shared_ptr<Texture> get3dTexture(const std::string & name,
				unsigned width, unsigned height, unsigned depth,
				const FlagSet & flags);

		/**
		 * get cube map
		 *
		 * @param name    texture name
		 * @param width   face width
		 * @param height  face height
		 * @param face    face index
		 *                0 all faces,
		 *                1 positive x,
		 *                2 negative x,
		 *                3 positive y,
		 *                4 negative y,
		 *                5 positive z,
		 *                6 negative z
		 * @return        cube map texture
		 */
		const std::shared_ptr<Texture> & getCubeMap(const std::string & name,
				unsigned width, unsigned height, Texture::Face face);

		const std::vector<std::shared_ptr<Texture>> & getCubeSet(int id);

		std::shared_ptr<Texture> getDepthTexture(const std::string & name,
				unsigned width, unsigned height, const std::string & type);

		size_t getId(const std::string & name);

		const std::shared_ptr<Texture> & getImage(
				const std::string & currentDir, const std::string & name,
				bool genmipmap, bool filter, bool wrap);

		const std::string & getName(int tid);

		/**
		 * get screen placeholder
		 *
		 * @return  placeholder texture for screen
		 */
		std::shared_ptr<Texture> getScreen();

		std::shared_ptr<Texture> getTexture(const std::string & name,
				unsigned width, unsigned height, bool filter);

		/**
		 * set width & height for screen placeholder
		 *
		 * @param width   screen width
		 * @param height  screen height
		 */
		void setScreen(unsigned width, unsigned height);

		static TextureManager & getInstance();

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

