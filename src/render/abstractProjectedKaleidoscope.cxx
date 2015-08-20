#include "abstractProjectedKaleidoscope.h"

#include "../core/convexHull.h"
#include "../core/indexArray.h"
#include "../core/normalArray.h"
#include "../core/vec2.h"
#include "../core/vec3Array.h"

#include <array>

using namespace render;

namespace {

	/* for convex hulls */
	IndexArray indices = { 0, 1, 2, 0, 2, 3 };
	IndexArray planeIndices = { 0, 0 };
	IndexArray edgeIndices = { 0, 1, 0, 1, 1, 2, 0, 1, 2, 3, 0, 1, 3, 0, 0, 1 };
	IndexArray faceDirectionIndices = { 0 };
	IndexArray edgeDirectionIndices = { 0, 1 };

	ConvexHull genHull(float radius) {
		double halfRadius = radius / 2;

		Vec3Array vertices{
				Vec3(halfRadius, halfRadius, 0.0),
				Vec3(-halfRadius, halfRadius, 0.0),
				Vec3(-halfRadius, -halfRadius, 0.0),
				Vec3(halfRadius, -halfRadius, 0.0) };

		NormalArray normals{ Normal(0.f, 0.f, 1.f),	Normal(0.f, 0.f, -1.f) };

		return ConvexHull(vertices.getBounds(), vertices, indices,
				normals, planeIndices, edgeIndices,
				faceDirectionIndices, edgeDirectionIndices);
	}
}

struct AbstractProjectedKaleidoscope::impl {
	std::shared_ptr<Texture> texture;
	float radius;
	int numSegments;
	float alpha;
	float beta;
	std::array<Vec2, 3> uvs;
	ConvexHull convexHull;

	impl(const std::shared_ptr<Texture> & texture, int numSegments,
			float radius, float alpha, float beta, const Vec2 & uv0,
			const Vec2 & uv1, const Vec2 & uv2) :
					texture(texture),
					radius(radius),
					numSegments(numSegments),
					alpha(alpha),
					beta(beta),
					uvs(std::array<Vec2, 3>{ { uv0, uv1, uv2 } }),
					convexHull(genHull(radius)) {

	}
};

AbstractProjectedKaleidoscope::AbstractProjectedKaleidoscope(
		const std::shared_ptr<Texture> & texture, int numSegments, float radius,
		float alpha, float beta, const Vec2 & uv0, const Vec2 & uv1,
		const Vec2 & uv2) :
				pimpl(new impl(texture, numSegments, radius, alpha, beta, uv0,
						uv1, uv2)) {
}

/**
 * destructor
 */
AbstractProjectedKaleidoscope::~AbstractProjectedKaleidoscope() {
}

/**
 * get projected kaleidoscope alpha
 *
 * @return  projected kaleidoscope alpha
 */
float AbstractProjectedKaleidoscope::getAlpha() const {
	return pimpl->alpha;
}

/**
 * get projected kaleidoscope beta
 *
 * @return  projected kaleidoscope beta
 */
float AbstractProjectedKaleidoscope::getBeta() const {
	return pimpl->beta;
}

/**
 * get convex hull of abstract projected kaleidoscope
 *
 * @return  convex hull
 */
const ConvexHull & AbstractProjectedKaleidoscope::getConvexHull() const {
	return pimpl->convexHull;
}

/**
 * get number of segments in kaleidoscope
 *
 * @return  number of segments
 */
int AbstractProjectedKaleidoscope::getNumSegments() const {
	return pimpl->numSegments;
}

/**
 * get radius of kaledioscope
 *
 * @return  radius of kaleidoscope
 */
float AbstractProjectedKaleidoscope::getRadius() const {
	return pimpl->radius;
}

/**
 * get texture of kaledioscope
 *
 * @return  texture of kaleidoscope
 */
const std::shared_ptr<Texture> & AbstractProjectedKaleidoscope::getTexture() const {
	return pimpl->texture;
}

/**
 * get uvs of kaledioscope
 *
 * @return  uvs of kaleidoscope
 */
const std::array<Vec2, 3> & AbstractProjectedKaleidoscope::getUVs() const {
	return pimpl->uvs;
}
