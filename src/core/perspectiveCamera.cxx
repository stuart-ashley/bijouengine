#define _USE_MATH_DEFINES
#include "perspectiveCamera.h"

#include "indexArray.h"
#include "normalArray.h"
#include "vec2.h"

#include <cmath>

namespace {
	IndexArray indices = { 0, 3, 2, 0, 2, 1, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 5, 4,
			1, 2, 6, 1, 6, 5, 2, 3, 7, 2, 7, 6, 3, 0, 4, 3, 4, 7 };
	IndexArray planeIndices = { 0, 4, 0, 1, 2, 3 };
	IndexArray edgeIndices = { 0, 1, 0, 2, 1, 2, 0, 3, 2, 3, 0, 4, 3, 0, 0, 5,
			4, 5, 1, 2, 5, 6, 1, 3, 6, 7, 1, 4, 7, 4, 1, 5, 0, 4, 2, 5, 1, 5, 2,
			3, 2, 6, 3, 4, 3, 7, 4, 5 };
	IndexArray faceDirectionIndices = { 0, 2, 3, 4, 5 };
	IndexArray edgeDirectionIndices = { 0, 1, 1, 2, 0, 4, 1, 5, 2, 6, 3, 7 };

	/*
	 *
	 */
	Mat4 createProjectionMatrix(float fovyInDegrees, float aspectRatio,
			float near, float far) {
		float f = static_cast<float>(1 / std::tan(fovyInDegrees * M_PI / 360));

		return Mat4(f / aspectRatio, 0, 0, 0, 0, f, 0, 0, 0, 0,
				(far + near) / (near - far), (2 * far * near) / (near - far), 0,
				0, -1, 0);
	}

	/*
	 *
	 */
	ConvexHull createConvexHull(float fovyInDegrees, float aspectRatio,
			float near, float far) {
		float tanHalfFovy = static_cast<float>(std::tan(fovyInDegrees * M_PI / 360));
		double halfFovx = std::atan(tanHalfFovy * aspectRatio);
		double tanHalfFovx = std::tan(halfFovx);

		std::vector<Vec3> vertices;
		vertices.emplace_back(-tanHalfFovx * near, tanHalfFovy * near, -near);
		vertices.emplace_back(tanHalfFovx * near, tanHalfFovy * near, -near);
		vertices.emplace_back(tanHalfFovx * near, -tanHalfFovy * near, -near);
		vertices.emplace_back(-tanHalfFovx * near, -tanHalfFovy * near, -near);
		vertices.emplace_back(-tanHalfFovx * far, tanHalfFovy * far, -far);
		vertices.emplace_back(tanHalfFovx * far, tanHalfFovy * far, -far);
		vertices.emplace_back(tanHalfFovx * far, -tanHalfFovy * far, -far);
		vertices.emplace_back(-tanHalfFovx * far, -tanHalfFovy * far, -far);

		std::vector<Normal> faceNormals;
		faceNormals.emplace_back(0.f, 0.f, 1.f);
		faceNormals.emplace_back(0.f, 0.f, -1.f);

		Vec3 u, v, n;

		u = vertices[1] - vertices[0];
		v = vertices[4] - vertices[0];
		n.cross(u, v);
		faceNormals.emplace_back(n.normalized());

		u = vertices[2] - vertices[1];
		v = vertices[5] - vertices[1];
		n.cross(u, v);
		faceNormals.emplace_back(n.normalized());

		u = vertices[3] - vertices[2];
		v = vertices[6] - vertices[2];
		n.cross(u, v);
		faceNormals.emplace_back(n.normalized());

		u = vertices[0] - vertices[3];
		v = vertices[7] - vertices[3];
		n.cross(u, v);
		faceNormals.emplace_back(n.normalized());

		Vec3Array vertexArray(vertices);

		return ConvexHull(vertexArray.getBounds(), vertexArray, indices,
				NormalArray(faceNormals), planeIndices, edgeIndices,
				faceDirectionIndices, edgeDirectionIndices);
	}
}

/**
 * Construct a projective camera. Projection matrix generated is identical
 * to that produced by gluPerspective
 *
 * @param name           name of camera
 * @param fovyInDegrees  field of view in degrees
 * @param aspectRatio    aspect ratio of camera, horizontal / vertical length
 * @param near           near plane
 * @param far            far plane
 */
PerspectiveCamera::PerspectiveCamera(const std::string & name,
		float fovyInDegrees, float aspectRatio, float near, float far) :
				Camera(name, near, far, aspectRatio,
						createProjectionMatrix(fovyInDegrees, aspectRatio, near, far),
						createConvexHull(fovyInDegrees, aspectRatio, near, far)),
				fovyInDegrees(fovyInDegrees),
				tanHalfFovy(static_cast<float>(std::tan(fovyInDegrees * M_PI / 360))) {
}

/*
 *
 */
OVERRIDE std::shared_ptr<Camera> PerspectiveCamera::clone(
		const std::string & name) const {
	return std::make_shared<PerspectiveCamera>(name, fovyInDegrees, aspectRatio,
			near, far);
}

/*
 *
 */
OVERRIDE Vec3Array PerspectiveCamera::getCorners(double nearZ,
		double farZ) const {
	double nearY = nearZ * tanHalfFovy;
	double nearX = nearY * aspectRatio;
	double farY = farZ * tanHalfFovy;
	double farX = farY * aspectRatio;

	return Vec3Array{
			Vec3(nearX, nearY, -nearZ),
			Vec3(nearX, -nearY, -nearZ),
			Vec3(-nearX, nearY, -nearZ),
			Vec3(-nearX, -nearY, -nearZ),
			Vec3(farX, farY, -farZ),
			Vec3(farX, -farY, -farZ),
			Vec3(-farX, farY, -farZ),
			Vec3(-farX, -farY, -farZ) };
}

/**
 * calculate ray from camera in camera space
 *
 * @param pos  normalized position looking out from camera
 *
 * @return     ray from camera
 */
OVERRIDE Ray PerspectiveCamera::getRay(const Vec2 & pos) const {
	return Ray(Vec3(),
			Vec3((2 * pos.getX() - 1) * tanHalfFovy * aspectRatio,
					(2 * pos.getY() - 1) * tanHalfFovy, -1.0));
}
