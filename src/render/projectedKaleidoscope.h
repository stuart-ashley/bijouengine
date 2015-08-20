#pragma once

#include <array>
#include <memory>

class Aspect;
class ConvexHull;
class Mat4;
class Transform;
class Vec2;

namespace render {
	class AbstractProjectedKaleidoscope;
	class Spotlight;
	class Sunlight;
	class Texture;

	class ProjectedKaleidoscope {
	public:

		/**
		 * create kaleidoscope projection for spotlight
		 *
		 * @param projectedKaleidoscope  abstract kaleidoscope
		 * @param transform              kaleidoscope transform
		 * @param spot                   spotlight
		 */
		ProjectedKaleidoscope(
				const AbstractProjectedKaleidoscope & projectedKaleidoscope,
				const Transform & transform, const Spotlight & spot);

		/**
		 * create kaleidoscope projection for sunlight
		 *
		 * @param projectedKaleidoscope  abstract kaleidoscope
		 * @param transform              kaleidoscope transform
		 * @param light                  sunlight for projection
		 */
		ProjectedKaleidoscope(
				const AbstractProjectedKaleidoscope & projectedKaleidoscope,
				const Transform & transform, const Sunlight & light);

		/**
		 * get projected kaleidoscope alpha
		 *
		 * @return  projected kaleidoscope alpha
		 */
		float getAlpha() const;

		/**
		 * get projected kaleidoscope beta
		 *
		 * @return  projected kaleidoscope beta
		 */
		float getBeta() const;

		/**
		 *
		 *
		 * @return
		 */
		const ConvexHull & getConvexHull() const;

		/**
		 *
		 *
		 * @return
		 */
		float getSegmentCentralAngleRadians() const;

		/**
		 *
		 * @param aspect  current aspect
		 *
		 * @return
		 */
		Mat4 getTexMat(const Aspect & aspect) const;

		/**
		 * get projected kaleidoscope texture
		 *
		 * @return  projected kaleidoscope texture
		 */
		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 *
		 *
		 * @return
		 */
		const Transform & getTransform() const;

		/**
		 * get projected kaleidoscope uvs
		 *
		 * @return  projected kaleidoscope uvs
		 */
		const std::array<Vec2, 3> & getUVs() const;

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

