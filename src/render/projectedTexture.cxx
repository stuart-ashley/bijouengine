#include "projectedTexture.h"

#include "abstractProjectedTexture.h"
#include "spotlight.h"
#include "sunlight.h"

#include "../core/aspect.h"
#include "../core/convexHull.h"
#include "../core/indexArray.h"
#include "../core/mat4.h"
#include "../core/normalArray.h"
#include "../core/transform.h"

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
	 * calculate texture matrix for texture for spotlight
	 *
	 * @param spot                      spotlight
	 * @param transform                 texture transform
	 * @param abstractProjectedTexture  projected texture
	 *
	 * @return                          texture matrix
	 */
	Mat4 textureMatrixForSpotlight(const Spotlight & spot,
			const Transform & transform,
			const AbstractProjectedTexture & abstractProjectedTexture) {
		Vec3 p = spot.getTransform().getTranslation();
		transform.inverseTransformPoint(p);

		// row major, texture matrix
		Mat4 texmat(.5f, 0, 0, .5f, 0, .5f, 0, .5f, 0, 0, .5f, .5f, 0, 0, 0, 1);

		// frustum projection matrix
		float x = static_cast<float>(p.getX());
		float y = static_cast<float>(p.getY());
		float n = static_cast<float>(p.getZ());
		float f = far + n;
		float w = -abstractProjectedTexture.getWidth();
		float h = abstractProjectedTexture.getHeight();
		texmat.mul(
				Mat4(2 * n / w, 0, -2 * x / w, 0, 0, 2 * n / h, -2 * y / h, 0,
						0, 0, (f + n) / (n - f), 2 * f * n / (n - f), 0, 0, -1,
						0));
		// from p
		texmat.mul(Mat4(1, 0, 0, -x, 0, 1, 0, -y, 0, 0, 1, -n, 0, 0, 0, 1));

		return texmat;
	}

	/**
	 * calculate convex hull for texture for spotlight
	 *
	 * @param spot                      spotlight
	 * @param transform                 texture transform
	 * @param abstractProjectedTexture  projected texture
	 *
	 * @return                          texture matrix
	 */
	ConvexHull convexHullForSpotlight(const Spotlight & spot,
			const Transform & transform,
			const AbstractProjectedTexture & abstractProjectedTexture) {
		Vec3 p = spot.getTransform().getTranslation();
		transform.inverseTransformPoint(p);

		double hw = abstractProjectedTexture.getWidth() / 2;
		double hh = abstractProjectedTexture.getHeight() / 2;

		std::vector<Vec3> vertices;
		Vec3 tmp[] = { Vec3(-hw, -hh, 0.0), Vec3(-hw, hh, 0.0), Vec3(hw, -hh,
				0.0), Vec3(hw, hh, 0.0) };
		for (Vec3 q : tmp) {
			Vec3 r = q - p;
			r.scaleAdd(-far / (r.getZ() * r.length()), r, q);
			vertices.emplace_back(q);
			vertices.emplace_back(r);
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
	 * calculate texture matrix for texture for sunlight
	 *
	 * @param light                     sunlight
	 * @param transform                 texture transform
	 * @param abstractProjectedTexture  projected texture
	 *
	 * @return                          texture matrix
	 */
	Mat4 textureMatrixForSunlight(const Sunlight & light,
			const Transform & transform,
			const AbstractProjectedTexture & abstractProjectedTexture) {
		Normal d(light.getDirection());
		transform.rotateInverse(d);

		// row major, texture matrix
		Mat4 texmat(.5f, 0, 0, .5f, 0, .5f, 0, .5f, 0, 0, 1, 0, 0, 0, 0, 1);

		// orthographic projection matrix
		float w = -abstractProjectedTexture.getWidth();
		float h = abstractProjectedTexture.getHeight();
		texmat.mul(
				Mat4(2 / w, 0, -2 * d.getX() / (w * d.getZ()), 0, 0, 2 / h,
						-2 * d.getY() / (h * d.getZ()), 0, 0, 0, -1 / far, 0, 0,
						0, 0, 1));

		return texmat;
	}

	/**
	 * calculate convex hull for texture for sunlight
	 *
	 * @param light                     sunlight
	 * @param transform                 texture transform
	 * @param abstractProjectedTexture  projected texture
	 *
	 * @return                          texture matrix
	 */
	ConvexHull convexHullForSunlight(const Sunlight & light,
			const Transform & transform,
			const AbstractProjectedTexture & abstractProjectedTexture) {
		Normal d(light.getDirection());
		transform.rotateInverse(d);

		Vec3 fd;
		fd.scale(far / -d.getZ(), d);
		double hw = abstractProjectedTexture.getWidth() / 2;
		double hh = abstractProjectedTexture.getHeight() / 2;

		std::vector<Vec3> vertices;
		Vec3 tmp[] = { Vec3(-hw, -hh, 0.0), Vec3(-hw, hh, 0.0), Vec3(hw, -hh,
				0.0), Vec3(hw, hh, 0.0) };
		for (Vec3 q : tmp) {
			Vec3 r = q + fd;
			vertices.emplace_back(q);
			vertices.emplace_back(r);
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

struct ProjectedTexture::impl {
	AbstractProjectedTexture abstractProjectedTexture;
	Transform transform;
	/** world coordinates to texture space */
	Mat4 texmat;
	/** projected texture space convex hull */
	ConvexHull convexHull;

	impl(const AbstractProjectedTexture & abstractProjectedTexture,
			const Transform & transform, const Spotlight & spot) :
					abstractProjectedTexture(abstractProjectedTexture),
					transform(transform),
					texmat(textureMatrixForSpotlight(spot, transform,
							abstractProjectedTexture)),
					convexHull(convexHullForSpotlight(spot, transform,
							abstractProjectedTexture)) {
	}

	impl(const AbstractProjectedTexture & abstractProjectedTexture,
			const Transform & transform, const Sunlight & sun) :
					abstractProjectedTexture(abstractProjectedTexture),
					transform(transform),
					texmat(textureMatrixForSunlight(sun, transform,
							abstractProjectedTexture)),
					convexHull(convexHullForSunlight(sun, transform,
							abstractProjectedTexture)) {
	}
};

/**
 *
 * @param abstractProjectedTexture
 * @param transform
 * @param spot                      spotlight
 */
ProjectedTexture::ProjectedTexture(
		const AbstractProjectedTexture & abstractProjectedTexture,
		const Transform & transform, const Spotlight & spot) :
		pimpl(new impl(abstractProjectedTexture, transform, spot)) {
}

/**
 *
 * @param abstractProjectedTexture
 * @param transform
 * @param light                     directional light
 */
ProjectedTexture::ProjectedTexture(
		const AbstractProjectedTexture & abstractProjectedTexture,
		const Transform & transform, const Sunlight & light) :
		pimpl(new impl(abstractProjectedTexture, transform, light)) {
}

/**
 * get projected texture alpha
 *
 * @return  projected texture alpha
 */
float ProjectedTexture::getAlpha() const {
	return pimpl->abstractProjectedTexture.getAlpha();
}

/**
 * get projected texture beta
 *
 * @return  projected texture beta
 */
float ProjectedTexture::getBeta() const {
	return pimpl->abstractProjectedTexture.getBeta();
}

/**
 *
 * @return
 */
const ConvexHull & ProjectedTexture::getConvexHull() const {
	return pimpl->convexHull;
}

/**
 *
 * @param aspect  current aspect
 *
 * @return
 */
Mat4 ProjectedTexture::getTexMat(const Aspect & aspect) const {
	Mat4 m = aspect.getRotTrans().to(pimpl->transform).asMat4();
	m.mul(pimpl->texmat, m);
	return m;
}

/**
 *
 * @return
 */
const std::shared_ptr<Texture> & ProjectedTexture::getTexture() const {
	return pimpl->abstractProjectedTexture.getTexture();
}

/**
 *
 * @return
 */
const Transform & ProjectedTexture::getTransform() const {
	return pimpl->transform;
}
