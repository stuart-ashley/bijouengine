#define _USE_MATH_DEFINES
#include "abstractSpotlight.h"

#include "indexArray.h"
#include "normalArray.h"
#include "vec3Array.h"

namespace {
	IndexArray indices =
			{ 0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 1, 4, 1, 2, 3, 1, 3, 4 };
	IndexArray planeIndices = { 0, 0, 0, 0, 1 };
	IndexArray edgeIndices = { 0, 1, 0, 3, 0, 2, 0, 1, 0, 3, 1, 2, 0, 4, 2, 3,
			1, 2, 0, 4, 2, 3, 1, 4, 3, 4, 2, 4, 4, 1, 3, 4 };
	IndexArray faceDirectionIndices = { 0, 1, 2, 3, 4 };
	IndexArray edgeDirectionIndices = { 0, 1, 0, 2, 0, 3, 0, 4, 1, 2, 2, 3 };

	ConvexHull createConvexHull(float distance, float halfRadians) {
		float s = std::sin(halfRadians);
		float c = std::cos(halfRadians);
		double td = std::tan(halfRadians) * distance;

		Vec3Array vertices{
				Vec3(),
				Vec3(-td, td, -distance),
				Vec3(td, td, -distance),
				Vec3(td, -td, -distance),
				Vec3(-td, -td, -distance) };

		NormalArray faceNormals{
				Normal(0.f, c, s),
				Normal(c, 0.f, s),
				Normal(0.f, -c, s),
				Normal(-c, 0.f, s),
				Normal(0.f, 0.f, -1.f) };

		return ConvexHull(vertices.getBounds(), vertices, indices,
				faceNormals, planeIndices, edgeIndices,
				faceDirectionIndices, edgeDirectionIndices);
	}
}

/**
 * construct spotlight
 *
 * @param name      name of spotlight
 * @param color     color of spotlight
 * @param distance  distance of spotlight
 * @param angle     angle of spotlight
 * @param blend     amount of blend
 */
AbstractSpotlight::AbstractSpotlight(const std::string & name,
		const Color & color, float distance, float angle, float blend) :
				AbstractLight(name, color),
				distance(distance),
				halfRadians(static_cast<float>(angle / 360 * M_PI)),
				blend(blend),
				convexHull(createConvexHull(distance, halfRadians)) {
}

/**
 * get spotlight angle
 *
 * @return  spotlight angle
 */
float AbstractSpotlight::getAngle() const {
	return static_cast<float>(halfRadians * 360 / M_PI);
}

/**
 * get spotlight convex hull
 *
 * @return  convex hull of spotlight
 */
const ConvexHull & AbstractSpotlight::getConvexHull() const {
	return convexHull;
}

/**
 * get spotlight distance
 *
 * @return  spotlight distance
 */
float AbstractSpotlight::getDistance() const {
	return distance;
}

/**
 * get half angle in radians
 *
 * @return  half angle in radians
 */
float AbstractSpotlight::getHalfAngleRadians() const {
	return halfRadians;
}
