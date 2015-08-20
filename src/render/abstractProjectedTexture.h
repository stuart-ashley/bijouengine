#pragma once

#include <memory>

class ConvexHull;

namespace render {
	class Texture;

	class AbstractProjectedTexture {
	public:

		AbstractProjectedTexture(const std::shared_ptr<Texture> & texture,
				float width, float height, float alpha, float beta);

		/**
		 * get projected texture alpha
		 *
		 * @return  projected texture alpha
		 */
		float getAlpha() const;

		/**
		 * get projected texture beta
		 *
		 * @return  projected texture beta
		 */
		float getBeta() const;

		const ConvexHull & getConvexHull() const;

		/**
		 * get projected texture height
		 *
		 * @return  projected texture height
		 */
		float getHeight() const;

		/**
		 * get projected texture
		 *
		 * @return  projected texture
		 */
		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 * get projected texture width
		 *
		 * @return  projected texture width
		 */
		float getWidth() const;

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}
