#include "plane.h"

#include "boundingBox.h"
#include "transform.h"
#include "vec3Array.h"

/*
 *
 */
double Plane::distanceTo(const Vec3 & p) const {
	return normal.getX() * (p.getX() - point.getX())
			+ normal.getY() * (p.getY() - point.getY())
			+ normal.getZ() * (p.getZ() - point.getZ());
}

/*
 *
 */
bool Plane::isAbove(const BoundingBox & box) const {
	const auto & min = box.getMin();
	double ax = (min.getX() - point.getX()) * normal.getX();
	double ay = (min.getY() - point.getY()) * normal.getY();
	double az = (min.getZ() - point.getZ()) * normal.getZ();

	if (ax + ay + az >= 0) {
		return false;
	}

	const auto & max = box.getMax();
	double bx = (max.getX() - point.getX()) * normal.getX();
	double by = (max.getY() - point.getY()) * normal.getY();
	double bz = (max.getZ() - point.getZ()) * normal.getZ();

	if (ax + ay + bz >= 0 || ax + by + az >= 0 || ax + by + bz >= 0
			|| bx + ay + az >= 0 || bx + ay + bz >= 0 || bx + by + az >= 0
			|| bx + by + bz >= 0) {
		return false;
	}

	return true;
}

/*
 *
 */
bool Plane::isAbove(const Vec3Array & array) const {
	for (Vec3 v : array) {
		if (distanceTo(v) >= 0) {
			return false;
		}
	}
	return true;
}

/*
 *
 */
bool Plane::isBelow(const BoundingBox & box) const {
	const auto & min = box.getMin();
	double ax = (min.getX() - point.getX()) * normal.getX();
	double ay = (min.getY() - point.getY()) * normal.getY();
	double az = (min.getZ() - point.getZ()) * normal.getZ();

	if (ax + ay + az <= 0) {
		return false;
	}

	const auto & max = box.getMax();
	double bx = (max.getX() - point.getX()) * normal.getX();
	double by = (max.getY() - point.getY()) * normal.getY();
	double bz = (max.getZ() - point.getZ()) * normal.getZ();

	if (ax + ay + bz <= 0 || ax + by + az <= 0 || ax + by + bz <= 0
			|| bx + ay + az <= 0 || bx + ay + bz <= 0 || bx + by + az <= 0
			|| bx + by + bz <= 0) {
		return false;
	}

	return true;
}

/*
 *
 */
bool Plane::isBelow(const Vec3 & p) const {
	return distanceTo(p) > 0;
}

/*
 *
 */
bool Plane::isBelow(const Vec3Array & array) const {
	for (Vec3 v : array) {
		if (distanceTo(v) <= 0) {
			return false;
		}
	}
	return true;
}

/*
 *
 */
void Plane::transform(const Transform & transform)  {
	transform.transformPoint(point);
	transform.rotate(normal);
}
