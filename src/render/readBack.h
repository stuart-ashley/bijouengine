#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace render {
	class Texture;
	class Canvas;

	class ReadBack {
	public:

		/**
		 *
		 * @param texture
		 * @param x
		 * @param y
		 * @param width
		 * @param height
		 */
		ReadBack(const std::shared_ptr<Texture> & texture, int x, int y,
				int width, int height);

		/**
		 * destructor
		 */
		~ReadBack();

		/**
		 *
		 */
		void execute(Canvas & canvas);

		/**
		 *
		 * @return
		 */
		const std::vector<uint8_t> & getBytes() const;

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

