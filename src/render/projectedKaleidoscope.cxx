#define _USE_MATH_DEFINES
#include "projectedKaleidoscope.h"

#include "abstractProjectedKaleidoscope.h"
#include "spotlight.h"
#include "sunlight.h"

#include "../core/aspect.h"
#include "../core/convexHull.h"
#include "../core/indexArray.h"
#include "../core/mat4.h"
#include "../core/normalArray.h"
#include "../core/transform.h"

#include <cmath>

using namespace render;

namespace {

	float far = 100;

	IndexArray indices = { 0, 1, 3, 0, 3, 2, 0, 4, 5, 0, 5, 1, 0, 2, 6, 0, 6, 4,
			1, 5, 7, 1, 7, 3, 4, 6, 7, 4, 7, 5, 2, 3, 7, 2, 7, 6 };
	IndexArray planeIndices = { 0, 0, 0, 7, 7, 7 };
	IndexArray edgeIndices = { 0, 1, 0, 1, 0, 2, 0, 2, 1, 3, 0, 3, 2, 3, 0, 5,
			0, 4, 1, 2, 1, 5, 1, 3, 4, 5, 1, 4, 4, 6, 2, 4, 2, 6, 2, 5, 5, 7, 3,
			4, 3, 7, 3, 5, 6, 7, 4, 5 };
	IndexArray orthoFaceDirectionIndices = { 0, 1, 2 };
	IndexArray orthoEdgeDirectionIndices = { 0, 1, 4 };

	/**
	 * calculate texture matrix for kaleidoscope for spotlight
	 *
	 * @param spot       spotlight
	 * @param transform  kaleidoscope transform
	 * @param radius     kaleidoscope radius
	 *
	 * @return           texture matrix
	 */
	Mat4 textureMatrixForSpotlight(const Spotlight & spot,
			const Transform & transform, float radius) {
		Vec3 p = spot.getTransform().getTranslation();
		transform.inverseTransformPoint(p);

		// row major, texture matrix
		Mat4 texmat(.5f, 0, 0, .5f, 0, .5f, 0, .5f, 0, 0, .5f, .5f, 0, 0, 0, 1);
		// frustum projection matrix
		float x = static_cast<float>(p.getX());
		float y = static_cast<float>(p.getY());
		float n = static_cast<float>(p.getZ());
		float f = far + n;
		float r = -radius;
		texmat.mul(
				Mat4(2 * n / r, 0, -2 * x / r, 0, 0, 2 * n / r, -2 * y / r, 0,
						0, 0, (f + n) / (n - f), 2 * f * n / (n - f), 0, 0, -1,
						0));
		// from p
		texmat.mul(Mat4(1, 0, 0, -x, 0, 1, 0, -y, 0, 0, 1, -n, 0, 0, 0, 1));

		return texmat;
	}

	/**
	 * calculate convex hull for kaleidoscope for spotlight
	 *
	 * @param spot       spotlight
	 * @param transform  kaleidoscope transform
	 * @param radius     kaleidoscope radius
	 *
	 * @return           texture matrix
	 */
	ConvexHull convexHullForSpotlight(const Spotlight & spot,
			const Transform & transform, float radius) {
		Vec3 p = spot.getTransform().getTranslation();
		transform.inverseTransformPoint(p);

		double hr = radius / 2;

		std::vector<Vec3> vertices;
		Vec3 tmp[] = { Vec3(-hr, -hr, 0.0), Vec3(-hr, hr, 0.0), Vec3(hr, -hr,
				0.0), Vec3(hr, hr, 0.0) };
		for (const auto & q : tmp) {
			Vec3 v = q - p;
			v.scaleAdd(-far / (v.getZ() * v.length()), v, q);
			vertices.emplace_back(q);
			vertices.emplace_back(v);
		}

		std::vector<Normal> normals;
		short idxs[18] =
				{ 0, 1, 3, 0, 4, 5, 0, 2, 6, 1, 5, 7, 4, 6, 7, 2, 3, 7 };
		for (int i = 0; i < 18; i += 3) {
			Vec3 aa = vertices.at(idxs[i + 1]) - vertices.at(idxs[i]);
			Vec3 bb = vertices.at(idxs[i + 2]) - vertices.at(idxs[i + 1]);
			aa.cross(aa, bb);
			normals.emplace_back(aa.normalized());
		}

		Vec3Array vertexArray(vertices);

		return ConvexHull(vertexArray.getBounds(), vertexArray, indices,
				NormalArray(normals), planeIndices, edgeIndices,
				orthoFaceDirectionIndices, orthoEdgeDirectionIndices);
	}

	/**
	 * calculate texture matrix for kaleidoscope for sunlight
	 *
	 * @param light      sunlight
	 * @param transform  kaleidoscope transform
	 * @param radius     kaleidoscope radius
	 *
	 * @return           texture matrix
	 */
	Mat4 textureMatrixForSunlight(const Sunlight & light,
			const Transform & transform, float radius) {
		Normal d = light.getDirection();
		transform.rotateInverse(d);

		// row major, texture matrix
		Mat4 texmat(.5f, 0, 0, .5f, 0, .5f, 0, .5f, 0, 0, 1, 0, 0, 0, 0, 1);
		// orthographic projection matrix
		float r = -radius;
		texmat.mul(
				Mat4(2 / r, 0, -2 * d.getX() / (r * d.getZ()), 0, 0, 2 / r,
						-2 * d.getY() / (r * d.getZ()), 0, 0, 0, -1 / far, 0, 0,
						0, 0, 1));
		return texmat;
	}

	/**
	 * calculate convex hull for kaleidoscope for sunlight
	 *
	 * @param light      sunlight
	 * @param transform  kaleidoscope transform
	 * @param radius     kaleidoscope radius
	 *
	 * @return           texture matrix
	 */
	ConvexHull convexHullForSunlight(const Sunlight & light,
			const Transform & transform, float radius) {
		Normal d = light.getDirection();
		transform.rotateInverse(d);

		Vec3 fd;
		fd.scale(far / d.getZ(), d);

		std::vector<Vec3> vertices;
		Vec3 tmp[] = { Vec3(-radius, -radius, 0.0), Vec3(-radius, radius, 0.0),
				Vec3(radius, -radius, 0.0), Vec3(radius, radius, 0.0) };
		for (const auto & q : tmp) {
			Vec3 v = q + fd;
			vertices.emplace_back(q);
			vertices.emplace_back(v);
		}

		std::vector<Normal> normals;
		short idxs[18] =
				{ 0, 1, 3, 0, 4, 5, 0, 2, 6, 1, 5, 7, 4, 6, 7, 2, 3, 7 };
		for (int i = 0; i < 18; i += 3) {
			Vec3 aa = vertices.at(idxs[i + 1]) - vertices.at(idxs[i]);
			Vec3 bb = vertices.at(idxs[i + 2]) - vertices.at(idxs[i + 1]);
			aa.cross(aa, bb);
			normals.emplace_back(aa.normalized());
		}

		Vec3Array vertexArray(vertices);

		return ConvexHull(vertexArray.getBounds(), vertexArray, indices,
				NormalArray(normals), planeIndices, edgeIndices,
				orthoFaceDirectionIndices, orthoEdgeDirectionIndices);
	}
}

struct ProjectedKaleidoscope::impl {
	AbstractProjectedKaleidoscope abstractProjectedKaleidoscope;
	Transform transform;
	/** world coordinates to texture space */
	Mat4 texmat;
	/** projected kaleidoscope space convex hull */
	ConvexHull convexHull;

	impl(const AbstractProjectedKaleidoscope & projectedKaleidoscope,
			const Transform & transform, const Spotlight & spot) :
					abstractProjectedKaleidoscope(projectedKaleidoscope),
					transform(transform),
					texmat(textureMatrixForSpotlight(spot, transform,
							projectedKaleidoscope.getRadius())),
					convexHull(convexHullForSpotlight(spot, transform,
							projectedKaleidoscope.getRadius())) {
	}

	impl(const AbstractProjectedKaleidoscope & projectedKaleidoscope,
			const Transform & transform, const Sunlight & sun) :
					abstractProjectedKaleidoscope(projectedKaleidoscope),
					transform(transform),
					texmat(textureMatrixForSunlight(sun, transform,
							projectedKaleidoscope.getRadius())),
					convexHull(convexHullForSunlight(sun, transform,
							projectedKaleidoscope.getRadius())) {
	}
};

/**
 * create kaleidoscope projection for spotlight
 *
 * @param projectedKaleidoscope  abstract kaleidoscope
 * @param transform              kaleidoscope transform
 * @param spot                   spotlight
 */
ProjectedKaleidoscope::ProjectedKaleidoscope(
		const AbstractProjectedKaleidoscope & projectedKaleidoscope,
		const Transform & transform, const Spotlight & spot) :
		pimpl(new impl(projectedKaleidoscope, transform, spot)) {
}

/**
 * create kaleidoscope projection for sunlight
 *
 * @param projectedKaleidoscope  abstract kaleidoscope
 * @param transform              kaleidoscope transform
 * @param light                  sunlight for projection
 */
ProjectedKaleidoscope::ProjectedKaleidoscope(
		const AbstractProjectedKaleidoscope & projectedKaleidoscope,
		const Transform & transform, const Sunlight & light) :
		pimpl(new impl(projectedKaleidoscope, transform, light)) {
}

/**
 * get projected kaleidoscope alpha
 *
 * @return  projected kaleidoscope alpha
 */
float ProjectedKaleidoscope::getAlpha() const {
	return pimpl->abstractProjectedKaleidoscope.getAlpha();
}

/**
 * get projected kaleidoscope beta
 *
 * @return  projected kaleidoscope beta
 */
float ProjectedKaleidoscope::getBeta() const {
	return pimpl->abstractProjectedKaleidoscope.getBeta();
}

/**
 *
 *
 * @return
 */
const ConvexHull & ProjectedKaleidoscope::getConvexHull() const {
	return pimpl->convexHull;
}

/**
 *
 *
 * @return
 */
float ProjectedKaleidoscope::getSegmentCentralAngleRadians() const {
	return static_cast<float>(2 * M_PI
			/ pimpl->abstractProjectedKaleidoscope.getNumSegments());
}

/**
 *
 * @param aspect  current aspect
 *
 * @return
 */
Mat4 ProjectedKaleidoscope::getTexMat(const Aspect & aspect) const {
	Mat4 m = aspect.getRotTrans().to(pimpl->transform).asMat4();
	m.mul(pimpl->texmat, m);
	return m;
}

/**
 * get projected kaleidoscope texture
 *
 * @return  projected kaleidoscope texture
 */
const std::shared_ptr<Texture> & ProjectedKaleidoscope::getTexture() const {
	return pimpl->abstractProjectedKaleidoscope.getTexture();
}

/**
 *
 *
 * @return
 */
const Transform & ProjectedKaleidoscope::getTransform() const {
	return pimpl->transform;
}

/**
 * get projected kaleidoscope uvs
 *
 * @return  projected kaleidoscope uvs
 */
const std::array<Vec2, 3> & ProjectedKaleidoscope::getUVs() const {
	return pimpl->abstractProjectedKaleidoscope.getUVs();
}
