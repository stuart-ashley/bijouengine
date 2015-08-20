#pragma once

#include "boundingBox.h"
#include "normal.h"
#include "vec3.h"

class Mat3;

class Triangle {
public:
	/**
	 * construct triangle from three points
	 *
	 * @param v0  first vertex
	 * @param v1  second vertex
	 * @param v2  third vertex
	 */
	Triangle(const Vec3 & v0, const Vec3 & v1, const Vec3 & v2);

	/**
	 * construct triangle from three points and normal
	 *
	 * @param v0  first vertex
	 * @param v1  second vertex
	 * @param v2  third vertex
	 * @param n  triangle normal
	 */
	inline Triangle(const Vec3 & v0, const Vec3 & v1, const Vec3 & v2,
			const Normal & n) :
			m_v0(v0), m_v1(v1), m_v2(v2), m_normal(n) {
	}

	/**
	 * copy constructor
	 */
	Triangle(const Triangle &) = default;

	/**
	 * destructor
	 */
	~Triangle() = default;

	/**
	 * Is point in triangle, point should be approximately on plane of triangle
	 *
	 * @param p   point to test
	 *
	 * @return true if point on triangle, false otherwise
	 */
	bool contains(const Vec3 & p) const;

	/**
	 * calculate area of triangle
	 *
	 * ½|(v1-v0)×(v2-v1)|
	 *
	 * @return  area of triangle
	 */
	double getArea() const;

	/**
	 * calculate bounds of triangle
	 *
	 * @return  bounds of triangle
	 */
	BoundingBox getBounds() const;

	/**
	 * calculate centre of triangle
	 *
	 * @return  centre of triangle
	 */
	Vec3 getCentre() const;

	/**
	 * get triangle normal
	 *
	 * @return  normal to triangle
	 */
	inline const Normal & getNormal() const {
		return m_normal;
	}

	/**
	 * get first vertex
	 *
	 * @return first vertex
	 */
	inline const Vec3 & getVertex0() const {
		return m_v0;
	}

	/**
	 * get second vertex
	 *
	 * @return second vertex
	 */
	inline const Vec3 & getVertex1() const {
		return m_v1;
	}

	/**
	 * get third vertex
	 *
	 * @return third vertex
	 */
	inline const Vec3 & getVertex2() const {
		return m_v2;
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
	bool intersectRay(const Vec3 & p, const Normal & n, Vec3 & op,
			double & d) const;

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
	bool intersectRayExtended(const Vec3 & p, const Normal & n, Vec3 & op,
			double & d) const;

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
	bool intersects(const Vec3 & p0, const Vec3 & p1, Vec3 & p,
			double & d) const;

	/**
	 * Triangle intersect triangle
	 *
	 * @param other  triangle to test against
	 *
	 * @return       true if triangle intersect triangle, false otherwise
	 */
	bool intersects(const Triangle & that) const;

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
	bool intersectsExtendedLineSegment(const Vec3 & p0, const Vec3 & p1,
			Vec3 & p, double & d) const;

	/**
	 * Planar distance to point p
	 *
	 * @param p   point
	 *
	 * @return    distance to point
	 */
	double planarDistance(const Vec3 & p) const;

	/**
	 * calculate and return triangle rotated and translated
	 *
	 * @param r  rotation matrix to apply
	 * @param t  translation to apply
	 *
	 * @return   transformed triangle
	 */
	Triangle rotatedTranslated(const Mat3 & r, const Vec3 & t) const;

private:
	Vec3 m_v0;
	Vec3 m_v1;
	Vec3 m_v2;
	Normal m_normal; // (v1-v0)×(v2-v1)/|(v1-v0)×(v2-v1)|
};

