#include "aspect.h"

#include <cmath>

namespace {

	Mat4 scale[] = { Mat4(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1),
			Mat4(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1), Mat4(1, 0, 0,
					0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1) };

}

/**
 *
 * @param plane
 */
void Aspect::addClipPlane(const Plane & plane) {
	for (const auto & p : m_clipPlanes) {
		if (p == plane) {
			return;
		}
	}
	m_clipPlanes.emplace_back(plane);
}

/**
 *
 * @param m
 */
void Aspect::transform(const Mat4 & m) {
	m_transform.mul(m);
	m_camera->transform(m);
	// inverse transform
	Mat4 inverse = m.inverse();
	// transform hull by inverse transform
	m_convexHull = m_convexHull.transformed(inverse);
}

/**
 *
 * @param plane
 */
void Aspect::mirror(const Plane & plane) {
	// plane in camera space
	Plane p = plane;
	p.transform(m_rotTrans.inverse());

	const auto & point = p.getPoint();
	const auto & normal = p.getNormal();

	// rotation n to y, (vx / e, vy / e, vz / e, e / 2)
	// where v = (n × y) and e = √(2(1 + n·y))
	Vec3 axis;
	Mat4 scaleMatrix;
	if (std::abs(normal.getY()) > std::abs(normal.getX())) {
		if (std::abs(normal.getZ()) > std::abs(normal.getY())) {
			axis.set(0, 0, 1);
			scaleMatrix = scale[2];
		} else {
			axis.set(0, 1, 0);
			scaleMatrix = scale[1];
		}
	} else if (std::abs(normal.getZ()) > std::abs(normal.getX())) {
		axis.set(0, 0, 1);
		scaleMatrix = scale[2];
	} else {
		axis.set(1, 0, 0);
		scaleMatrix = scale[0];
	}
	Vec3 v;
	v.cross(normal, axis);
	float e = static_cast<float>(std::sqrt(2 * (1 + axis.dot(normal))));
	Quat q(static_cast<float>(v.getX() / e), static_cast<float>(v.getY() / e),
			static_cast<float>(v.getZ() / e), e / 2);

	// rotation y to n
	Quat iq = q.conjugate();

	// calculate reflection matrix
	// T(p) * R(y,n)
	Mat4 reflectionMatrix(iq, point);
	// T(p) * R(y,n) * S(1,-1,1)
	reflectionMatrix.mul(scaleMatrix);
	// T(p) * R(y,n) * S(1,-1,1) * R(n,y) * T(-p)
	v = q.rotate(-point);
	reflectionMatrix.mul(Mat4(q, v));

	// apply reflection matrix
	transform(reflectionMatrix);
}
