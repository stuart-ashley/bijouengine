#pragma once

#include <memory>
#include <vector>

namespace render {
	class Texture;

	class Canvas {
	public:

		/**
		 * constructor
		 */
		Canvas();

		/**
		 * destructor
		 */
		~Canvas();

		/**
		 *
		 * @param texture
		 * @return
		 */
		bool setTarget(const std::shared_ptr<Texture> & texture);

		/**
		 *
		 * @param textures
		 * @return
		 */
		bool setTarget(const std::vector<std::shared_ptr<Texture>> & textures);

		/**
		 *
		 * @param textures
		 * @param layer
		 * @return
		 */
		bool setTarget(const std::vector<std::shared_ptr<Texture>> & textures,
				int layer);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

