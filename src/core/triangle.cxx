#include "triangle.h"

#include "transform.h"

#include <cmath>

namespace {
	const double epsilon = .00001f;
}

/**
 * construct triangle from three points
 *
 * @param v0  first vertex
 * @param v1  second vertex
 * @param v2  third vertex
 */
Triangle::Triangle(const Vec3 & v0, const Vec3 & v1, const Vec3 & v2) :
				m_v0(v0),
				m_v1(v1),
				m_v2(v2),
				m_normal((v1 - v0).cross(v2 - v1).normalized()) {
}

/**
 * Is point in triangle, point should be approximately on plane of triangle
 *
 * @param p   point to test
 *
 * @return true if point on triangle, false otherwise
 */
bool Triangle::contains(const Vec3 & p) const {
	Vec3 a = m_v2 - m_v0;
	Vec3 b = m_v1 - m_v0;
	Vec3 c = p - m_v0;

	double aa = a.dot(a);
	double ab = a.dot(b);
	double ac = a.dot(c);
	double bb = b.dot(b);
	double bc = b.dot(c);

	double i = 1 / (aa * bb - ab * ab);
	double u = (bb * ac - ab * bc) * i;
	double v = (aa * bc - ab * ac) * i;

	return (u >= 0) && (v >= 0) && (u + v <= 1);
}

/**
 * calculate area of triangle
 *
 * ½|(v1-v0)×(v2-v1)|
 *
 * @return  area of triangle
 */
double Triangle::getArea() const {
	Vec3 a = m_v1 - m_v0;
	Vec3 b = m_v2 - m_v1;
	return .5f * a.cross(b).length();
}

/**
 * calculate bounds of triangle
 *
 * @return  bounds of triangle
 */
BoundingBox Triangle::getBounds() const {
	return BoundingBox(m_v0, m_v1, m_v2);
}

/**
 * calculate centre of triangle
 *
 * @return  centre of triangle
 */
Vec3 Triangle::getCentre() const {
	return (m_v0 + m_v1 + m_v2) / 3;
}

/**
 * Ray intersection with both sides of a triangle, ray extends forward from
 * given point in given direction, there are no negative intersections
 *
 * @param p   ray point
 * @param n   ray direction
 * @param op  plane intersection, only if ray intersects triangle
 * @param d   distance to plane * length of ray direction, only if ray
 *            intersects triangle
 *
 * @return    true if ray intersects triangle, false otherwise
 */
bool Triangle::intersectRay(const Vec3 & p, const Normal & n, Vec3 & op,
		double & d) const {

	double d0 = (m_v0 - p).dot(m_normal);

	float d1 = n.dot(m_normal);

	// ray parallel with triangle
	if (d1 > -epsilon && d1 < epsilon) {
		return false;
	}

	double t = d0 / d1;
	if (t < 0.0) {
		return false;
	}

	Vec3 point;
	point.scaleAdd(t, n, p);

	if (contains(point) == false) {
		return false;
	}

	op = point;
	d = t;

	return true;
}

/**
 * Ray intersection with both sides of a triangle, ray extends forward from
 * given point in given direction, negative intersections included
 *
 * @param p   ray point
 * @param n   ray direction
 * @param op  plane intersection, only if ray intersects triangle
 * @param d   distance to plane * length of ray direction, only if ray
 *            intersects triangle
 *
 * @return    true if ray intersects triangle, false otherwise
 */
bool Triangle::intersectRayExtended(const Vec3 & p, const Normal & n, Vec3 & op,
		double & d) const {

	double d0 = (m_v0 - p).dot(m_normal);

	float d1 = n.dot(m_normal);

	// ray parallel with triangle
	if (d1 > -epsilon && d1 < epsilon) {
		return false;
	}

	double t = d0 / d1;

	Vec3 point;
	point.scaleAdd(t, n, p);

	if (contains(point) == false) {
		return false;
	}

	op = point;
	d = t;

	return true;
}

/**
 * Does line intersect triangle, and if so where
 *
 * @param p0  start of line
 * @param p1  end of line
 * @param p   point of intersection
 * @param d   intersection point as p0*(1 - d) + p1*d
 *
 * @return    true if there an intersection, false otherwise
 */
bool Triangle::intersects(const Vec3 & p0, const Vec3 & p1, Vec3 & p,
		double & d) const {

	// d0 = n·(p0-v0)
	double d0 = (p0 - m_v0).dot(m_normal);

	// d1 = n·(p1-v0)
	double d1 = (p1 - m_v0).dot(m_normal);

	// ensures t between 0 and 1
	if (d0 < 0.f && d1 < 0.f) {
		return false;
	}
	if (d0 > 0.f && d1 > 0.f) {
		return false;
	}

	double t = d0 / (d0 - d1);

	// p = p0 + (p1 - p0) * t
	Vec3 point = p1 - p0;
	point.scaleAdd(t, p0);

	if (contains(point) == false) {
		return false;
	}

	p = point;
	d = t;
	return true;
}

/**
 * Triangle intersect triangle
 *
 * @param other  triangle to test against
 *
 * @return       true if triangle intersect triangle, false otherwise
 */
bool Triangle::intersects(const Triangle & that) const {
	if (getBounds().intersects(that.getBounds()) == false) {
		return false;
	}

	Vec3 p;
	double d;

	if (intersects(that.m_v0, that.m_v1, p, d)) {
		return true;
	}
	if (intersects(that.m_v1, that.m_v2, p, d)) {
		return true;
	}
	if (intersects(that.m_v2, that.m_v0, p, d)) {
		return true;
	}

	if (that.intersects(m_v0, m_v1, p, d)) {
		return true;
	}
	if (that.intersects(m_v1, m_v2, p, d)) {
		return true;
	}
	if (that.intersects(m_v2, m_v0, p, d)) {
		return true;
	}

	return false;
}

/**
 * Does line segment intersect triangle, and if so where, segment is
 * extended forward and backward
 *
 * @param p0   start of line
 * @param p1   end of line
 * @param p    point of intersection
 * @param d    intersection point as p0*(1 - d) + p1*d
 *
 * @return     true if there an intersection, false otherwise
 */
bool Triangle::intersectsExtendedLineSegment(const Vec3 & p0, const Vec3 & p1,
		Vec3 & p, double & d) const {

	// d0 = n·(p0-v0)
	double d0 = (p0 - m_v0).dot(m_normal);

	// d1 = n·(p1-v0)
	double d1 = (p1 - m_v0).dot(m_normal);

	if (std::abs(d0 - d1) < epsilon) {
		return false;
	}

	double t = d0 / (d0 - d1);

	// p = p0 + (p1 - p0) * t
	Vec3 point = p1 - p0;
	point.scaleAdd(t, p0);

	if (contains(point) == false) {
		return false;
	}

	p = point;
	d = t;

	return true;
}

/**
 * Planar distance to point p
 *
 * @param p   point
 *
 * @return    distance to point
 */
double Triangle::planarDistance(const Vec3 & p) const {
	// n·(p0-v0)
	return (p - m_v0).dot(m_normal);
}

/**
 * calculate and return triangle rotated and translated
 *
 * @param r  rotation matrix to apply
 * @param t  translation to apply
 *
 * @return   transformed triangle
 */
Triangle Triangle::rotatedTranslated(const Mat3 & r, const Vec3 & t) const {
	return Triangle(r * m_v0 + t, r * m_v1 + t, r * m_v2 + t, r * m_normal);
}
