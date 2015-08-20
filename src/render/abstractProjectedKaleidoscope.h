#pragma once

#include "texture.h"

#include <array>
#include <memory>

class ConvexHull;
class Vec2;

namespace render {
	class Texture;

	class AbstractProjectedKaleidoscope {
	public:

		AbstractProjectedKaleidoscope(const std::shared_ptr<Texture> & texture,
				int numSegments, float radius, float alpha, float beta,
				const Vec2 & uv0, const Vec2 & uv1, const Vec2 & uv2);

		/**
		 * destructor
		 */
		~AbstractProjectedKaleidoscope();

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
		 * get convex hull of abstract projected kaleidoscope
		 *
		 * @return  convex hull
		 */
		const ConvexHull & getConvexHull() const;

		/**
		 * get number of segments in kaleidoscope
		 *
		 * @return  number of segments
		 */
		int getNumSegments() const;

		/**
		 * get radius of kaledioscope
		 *
		 * @return  radius of kaleidoscope
		 */
		float getRadius() const;

		/**
		 * get texture of kaledioscope
		 *
		 * @return  texture of kaleidoscope
		 */
		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 * get uvs of kaledioscope
		 *
		 * @return  uvs of kaleidoscope
		 */
		const std::array<Vec2, 3> & getUVs() const;

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

