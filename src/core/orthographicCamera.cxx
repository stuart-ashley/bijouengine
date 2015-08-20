#include "orthographicCamera.h"

#include "indexArray.h"
#include "normalArray.h"
#include "vec2.h"

namespace {

	IndexArray indices = { 0, 3, 2, 0, 2, 1, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 5, 4,
			1, 2, 6, 1, 6, 5, 2, 3, 7, 2, 7, 6, 3, 0, 4, 3, 4, 7 };
	IndexArray planeIndices = { 0, 4, 0, 1, 2, 3 };
	IndexArray edgeIndices = { 0, 1, 0, 2, 1, 2, 0, 3, 2, 3, 0, 4, 3, 0, 0, 5,
			4, 5, 1, 2, 5, 6, 1, 3, 6, 7, 1, 4, 7, 4, 1, 5, 0, 4, 2, 5, 1, 5, 2,
			3, 2, 6, 3, 4, 3, 7, 4, 5 };
	NormalArray faceNormals = {
			Normal(0.f, 0.f, 1.f),
			Normal(0.f, 0.f, -1.f),
			Normal(0.f, 1.f, 0.f),
			Normal(1.f, 0.f, 0.f),
			Normal(0.f, -1.f, 0.f),
			Normal(-1.f, 0.f, 0.f) };
	IndexArray faceDirectionIndices = { 0, 2, 3 };
	IndexArray edgeDirectionIndices = { 0, 1, 1, 2, 0, 4 };

	/*
	 *
	 */
	Mat4 createProjectionMatrix(float left, float right, float bottom,
			float top, float near, float far) {
		float tx = (right + left) / (left - right);
		float ty = (top + bottom) / (bottom - top);
		float tz = (far + near) / (near - far);

		// note the row major order
		return Mat4(2.f / (right - left), 0.f, 0.f, tx, 0.f,
				2.f / (top - bottom), 0.f, ty, 0.f, 0.f, 2.f / (near - far), tz,
				0.f, 0.f, 0.f, 1.f);
	}

	/*
	 *
	 */
	ConvexHull createConvexHull(float left, float right, float bottom,
			float top, float near, float far) {
		Vec3Array vertices{
				Vec3(left, top, -near),
				Vec3(right, top, -near),
				Vec3(right, bottom, -near),
				Vec3(left, bottom, -near),
				Vec3(left, top, -far),
				Vec3(right, top, -far),
				Vec3(right, bottom, -far),
				Vec3(left, bottom, -far) };

		return ConvexHull(vertices.getBounds(), vertices, indices, faceNormals,
				planeIndices, edgeIndices, faceDirectionIndices,
				edgeDirectionIndices);
	}
}

/**
 * Construct an orthographic camera. Projection matrix generated is
 * identical to one produced by glOrtho
 *
 * @param name    name of camera
 * @param left    left extents of view
 * @param right   right extents of view
 * @param bottom  bottom extent
 * @param top     top extent
 * @param near    near plane
 * @param far     far plane
 */
OrthographicCamera::OrthographicCamera(const std::string & name, float left,
		float right, float bottom, float top, float near, float far) :
				Camera(name, near, far, (right - left) / (top - bottom),
						createProjectionMatrix(left, right, bottom, top, near, far),
						createConvexHull(left, right, bottom, top, near, far)),
				left(left),
				right(right),
				bottom(bottom),
				top(top) {
}

OVERRIDE std::shared_ptr<Camera> OrthographicCamera::clone(
		const std::string & name) const {
	return std::make_shared<OrthographicCamera>(name, left, right, bottom, top,
			near, far);
}

OVERRIDE Vec3Array OrthographicCamera::getCorners(double nearZ,
		double farZ) const {
	return Vec3Array{
			Vec3(right, top, -nearZ),
			Vec3(right, bottom, -nearZ),
			Vec3(left, top, -nearZ),
			Vec3(left, bottom, -nearZ),
			Vec3(right, top, -farZ),
			Vec3(right, bottom, -farZ),
			Vec3(left, top, -farZ),
			Vec3(left, bottom, -farZ) };
}

OVERRIDE Ray OrthographicCamera::getRay(const Vec2 & pos) const {
	double px = pos.getX() * (right - left) + left;
	double py = pos.getY() * (top - bottom) + bottom;

	return Ray(Vec3(px, py, 0.0), Vec3(px, py, -1.0));
}
