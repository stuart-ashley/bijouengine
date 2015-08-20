#include "constraint.h"

/**
 *
 * @param b0
 * @param p0
 * @param r0
 * @param b1
 * @param p1
 * @param r1
 * @param limitFlags
 */
Constraint::Constraint(const std::string & b0, const Vec3 & p0, const Quat & r0,
		const std::string & b1, const Vec3 & p1, const Quat & r1,
		int limitFlags) :
				body0(b0),
				pivot0Pos(p0),
				pivot0Rot(r0),
				body1(b1),
				pivot1Pos(p1),
				pivot1Rot(r1),
				limitFlags(limitFlags),
				springyness(0),
				stiffness(0) {
}

/**
 *
 * @param b0
 * @param p0
 * @param r0
 * @param b1
 * @param p1
 * @param r1
 * @param limitFlags
 * @param minPos
 * @param maxPos
 * @param minRot
 * @param maxRot
 * @param springyness
 * @param stiffness
 */
Constraint::Constraint(const std::string & b0, const Vec3 & p0, const Quat & r0,
		const std::string & b1, const Vec3 & p1, const Quat & r1,
		int limitFlags, const Vec3 & minPos, const Vec3 & maxPos,
		const Vec3 & minRot, const Vec3 & maxRot, float springyness,
		float stiffness) :
				body0(b0),
				pivot0Pos(p0),
				pivot0Rot(r0),
				body1(b1),
				pivot1Pos(p1),
				pivot1Rot(r1),
				limitFlags(limitFlags),
				minPos(minPos),
				maxPos(maxPos),
				minRot(minRot),
				maxRot(maxRot),
				springyness(springyness),
				stiffness(stiffness) {
}

/**
 * Clamp position delta to limits
 *
 * @param delta  position delta to clamp
 *
 * @return       hit limit flags
 */
int Constraint::clampPositionDelta(Vec3 & delta) {
	int limited = 0;
	if ((limitFlags & 1) != 0) {
		if (minPos.getX() == maxPos.getX()) {
			limited |= 3;
			delta.setX(minPos.getX());
		} else if (delta.getX() < minPos.getX()) {
			limited |= 1;
			delta.setX(minPos.getX());
		} else if (delta.getX() > maxPos.getX()) {
			limited |= 2;
			delta.setX(maxPos.getX());
		}
	}
	if ((limitFlags & 2) != 0) {
		if (minPos.getY() == maxPos.getY()) {
			limited |= 12;
			delta.setY(minPos.getY());
		} else if (delta.getY() < minPos.getY()) {
			limited |= 4;
			delta.setY(minPos.getY());
		} else if (delta.getY() > maxPos.getY()) {
			limited |= 8;
			delta.setY(maxPos.getY());
		}
	}
	if ((limitFlags & 4) != 0) {
		if (minPos.getZ() == maxPos.getZ()) {
			limited |= 48;
			delta.setZ(minPos.getZ());
		} else if (delta.getZ() < minPos.getZ()) {
			limited |= 16;
			delta.setZ(minPos.getZ());
		} else if (delta.getZ() > maxPos.getZ()) {
			limited |= 32;
			delta.setZ(maxPos.getZ());
		}
	}
	return limited;
}

/**
 * Clamp rotation delta to limits
 *
 * @param euler  rotation to clamp
 *
 * @return       hit limit flags
 */
int Constraint::clampRotationDelta(Vec3 & euler) {
	int limited = 0;
	if ((limitFlags & 8) != 0) {
		if (minRot.getX() == maxRot.getX()) {
			limited |= 3;
			euler.setX(minRot.getX());
		} else if (euler.getX() < minRot.getX()) {
			limited |= 1;
			euler.setX(minRot.getX());
		} else if (euler.getX() > maxRot.getX()) {
			limited |= 2;
			euler.setX(maxRot.getX());
		}
	}
	if ((limitFlags & 16) != 0) {
		if (minRot.getY() == maxRot.getY()) {
			limited |= 12;
			euler.setY(minRot.getY());
		} else if (euler.getY() < minRot.getY()) {
			limited |= 4;
			euler.setY(minRot.getY());
		} else if (euler.getY() > maxRot.getY()) {
			limited |= 8;
			euler.setY(maxRot.getY());
		}
	}
	if ((limitFlags & 32) != 0) {
		if (minRot.getZ() == maxRot.getZ()) {
			limited |= 48;
			euler.setZ(minRot.getZ());
		} else if (euler.getZ() < minRot.getZ()) {
			limited |= 16;
			euler.setZ(minRot.getZ());
		} else if (euler.getZ() > maxRot.getZ()) {
			limited |= 32;
			euler.setZ(maxRot.getZ());
		}
	}
	return limited;
}
