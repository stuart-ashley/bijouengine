#pragma once

#include <memory>

class Aspect;
class ConvexHull;
class Mat4;
class Transform;

namespace render {
	class AbstractProjectedTexture;
	class Spotlight;
	class Sunlight;
	class Texture;

	class ProjectedTexture {
	public:

		/**
		 *
		 * @param abstractProjectedTexture
		 * @param transform
		 * @param spot                      spotlight
		 */
		ProjectedTexture(
				const AbstractProjectedTexture & abstractProjectedTexture,
				const Transform & transform, const Spotlight & spot);

		/**
		 *
		 * @param abstractProjectedTexture
		 * @param transform
		 * @param light                     directional light
		 */
		ProjectedTexture(
				const AbstractProjectedTexture & abstractProjectedTexture,
				const Transform & transform, const Sunlight & light);

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

		/**
		 *
		 * @return
		 */
		const ConvexHull & getConvexHull() const;

		/**
		 *
		 * @param aspect  current aspect
		 *
		 * @return
		 */
		Mat4 getTexMat(const Aspect & aspect) const;

		/**
		 *
		 * @return
		 */
		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 *
		 * @return
		 */
		const Transform & getTransform() const;

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

