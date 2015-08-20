#pragma once

#include "quat.h"
#include "vec3.h"

#include <string>

class Constraint {
public:

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
	Constraint(const std::string & b0, const Vec3 & p0, const Quat & r0,
			const std::string & b1, const Vec3 & p1, const Quat & r1,
			int limitFlags);

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
	Constraint(const std::string & b0, const Vec3 & p0, const Quat & r0,
			const std::string & b1, const Vec3 & p1, const Quat & r1,
			int limitFlags, const Vec3 & minPos, const Vec3 & maxPos,
			const Vec3 & minRot, const Vec3 & maxRot, float springyness,
			float stiffness);

	/**
	 * Clamp position delta to limits
	 *
	 * @param delta  position delta to clamp
	 *
	 * @return       hit limit flags
	 */
	int clampPositionDelta(Vec3 & delta);

	/**
	 * Clamp rotation delta to limits
	 *
	 * @param euler  rotation to clamp
	 *
	 * @return       hit limit flags
	 */
	int clampRotationDelta(Vec3 & euler);

	inline const std::string & getBodyName0() const {
		return body0;
	}

	inline const std::string & getBodyName1() const {
		return body1;
	}

	inline int getLimitFlags() const {
		return limitFlags;
	}

	inline const Vec3 & getPivot0Pos() const {
		return pivot0Pos;
	}

	inline const Quat & getPivot0Rot() const {
		return pivot0Rot;
	}

	inline const Vec3 & getPivot1Pos() const {
		return pivot1Pos;
	}

	inline const Quat & getPivot1Rot() const {
		return pivot1Rot;
	}

	inline void setBodyName0(const std::string & name) {
		body0 = name;
	}

	inline void setBodyName1(const std::string & name) {
		body1 = name;
	}

	inline void setPivot0Pos(const Vec3 & pos) {
		pivot0Pos = pos;
	}

	inline void setPivot0Rot(const Quat & rot) {
		pivot0Rot = rot;
	}

	inline void setPivot1Pos(const Vec3 & pos) {
		pivot1Pos = pos;
	}

	inline void setPivot1Rot(const Quat & rot) {
		pivot1Rot = rot;
	}

private:
	std::string body0;
	Vec3 pivot0Pos;
	Quat pivot0Rot;
	std::string body1;
	Vec3 pivot1Pos;
	Quat pivot1Rot;
	int limitFlags;
	Vec3 minPos;
	Vec3 maxPos;
	Vec3 minRot;
	Vec3 maxRot;
	float springyness;
	float stiffness;
};

