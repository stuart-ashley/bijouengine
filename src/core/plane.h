#pragma once

#include "normal.h"
#include "vec3.h"

class BoundingBox;
class Transform;
class Vec3Array;

class Plane {
public:
	/**
	 * constructor
	 *
	 * @param point   point on place
	 * @param normal  normal of plane
	 */
	Plane(const Vec3 & point, const Normal & normal) :
			point(point), normal(normal) {
	}

	/**
	 * default copy constructor
	 */
	Plane(const Plane &) = default;

	/**
	 * Distance from plane to point
	 *
	 * @param p  point to find distance to
	 *
	 * @return   distance to point, positive above plane, negative below plane
	 */
	double distanceTo(const Vec3 & p) const;

	/**
	 * negate plane normal
	 */
	void flipNormal() {
		normal = -normal;
	}

	/**
	 * get plane normal
	 *
	 * @return  normal to plane
	 */
	const Normal & getNormal() const {
		return normal;
	}

	/**
	 * get point on plane
	 *
	 * @return  point on plane
	 */
	const Vec3 & getPoint() const {
		return point;
	}

	/**
	 * Plane is above box ( box is below plane )
	 *
	 * @param box  box to check
	 *
	 * @return     true if box is entirely below plane
	 *             false if box is above or intersects or is on plane
	 */
	bool isAbove(const BoundingBox & box) const;

	/**
	 * Plane is above array of points ( array of points is below plane )
	 *
	 * @param array  array of points to compare
	 *
	 * @return       true if all points are below plane
	 *               false if any point is on plane or above plane
	 */
	bool isAbove(const Vec3Array & array) const;

	/**
	 * Plane is below box ( box is above plane )
	 *
	 * @param box  box to check
	 *
	 * @return     true if box is entirely above plane
	 *             false if box is below or intersects or is on plane
	 */
	bool isBelow(const BoundingBox & box) const;

	/**
	 * Plane is below point ( point is above plane )
	 *
	 * @param p  point to compare
	 *
	 * @return   true if point is above plane
	 *           false if point is on plane or under plane
	 */
	bool isBelow(const Vec3 & p) const;

	/**
	 * Plane is below array of points ( array of points is above plane )
	 *
	 * @param array  array of points to compare
	 *
	 * @return       true if all points are above plane
	 *               false if any point is on plane or under plane
	 */
	bool isBelow(const Vec3Array & array) const;

	inline bool operator==(const Plane & other) const {
		return point == other.point && normal == other.normal;
	}

	/**
	 * Transform plane by transform
	 *
	 * @param  transform
	 */
	void transform(const Transform & transform);

	/**
	 * representation of plane as string
	 *
	 * @return plane as string
	 */
	std::string toString() const {
		return "Plane(" + point.toString() + "," + normal.toString() + ")";
	}
private:
	Vec3 point;
	Normal normal;
};

