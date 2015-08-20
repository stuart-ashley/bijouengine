#include "abstractProjectedTexture.h"

#include "../core/convexHull.h"
#include "../core/indexArray.h"
#include "../core/normalArray.h"
#include "../core/vec3Array.h"

using namespace render;

namespace {

	/* for convex hulls */
	IndexArray indices = { 0, 1, 2, 0, 2, 3 };
	IndexArray planeIndices = { 0, 0 };
	IndexArray edgeIndices = { 0, 1, 0, 1, 1, 2, 0, 1, 2, 3, 0, 1, 3, 0, 0, 1 };
	IndexArray faceDirectionIndices = { 0 };
	IndexArray edgeDirectionIndices = { 0, 1 };

	ConvexHull makeConvexHull(float width, float height) {
		double halfWidth = width / 2;
		double halfHeight = height / 2;

		Vec3Array vertices{
				Vec3(halfWidth, halfHeight, 0.0),
				Vec3(-halfWidth, halfHeight, 0.0),
				Vec3(-halfWidth, -halfHeight, 0.0),
				Vec3(halfWidth, -halfHeight, 0.0) };

		NormalArray normals{ Normal(0.f, 0.f, 1.f), Normal(0.f, 0.f, -1.f) };

		return ConvexHull(vertices.getBounds(), vertices, indices,
				normals, planeIndices, edgeIndices,
				faceDirectionIndices, edgeDirectionIndices);
	}
}

struct AbstractProjectedTexture::impl {
	std::shared_ptr<Texture> texture;
	float width;
	float height;
	float alpha;
	float beta;
	ConvexHull convexHull;

	impl(const std::shared_ptr<Texture> & texture, float width, float height,
			float alpha, float beta) :
					texture(texture),
					width(width),
					height(height),
					alpha(alpha),
					beta(beta),
					convexHull(makeConvexHull(width, height)) {
	}
};

/*
 *
 */
AbstractProjectedTexture::AbstractProjectedTexture(
		const std::shared_ptr<Texture> & texture, float width, float height,
		float alpha, float beta) :
		pimpl(new impl(texture, width, height, alpha, beta)) {
}

/**
 * get projected texture alpha
 *
 * @return  projected texture alpha
 */
float AbstractProjectedTexture::getAlpha() const {
	return pimpl->alpha;
}

/**
 * get projected texture beta
 *
 * @return  projected texture beta
 */
float AbstractProjectedTexture::getBeta() const {
	return pimpl->beta;
}

/*
 *
 */
const ConvexHull & AbstractProjectedTexture::getConvexHull() const {
	return pimpl->convexHull;
}

/**
 * get projected texture height
 *
 * @return  projected texture height
 */
float AbstractProjectedTexture::getHeight() const {
	return pimpl->height;
}

/**
 * get projected texture
 *
 * @return  projected texture
 */
const std::shared_ptr<Texture> & AbstractProjectedTexture::getTexture() const {
	return pimpl->texture;
}

/**
 * get projected texture width
 *
 * @return  projected texture width
 */
float AbstractProjectedTexture::getWidth() const {
	return pimpl->width;
}
