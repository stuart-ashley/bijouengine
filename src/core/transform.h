#pragma once

#include "mat3.h"
#include "mat4.h"
#include "quat.h"
#include "vec3.h"

#include "../scripting/scriptObject.h"

class Transform final: public ScriptObject {
public:

	Transform(const Transform &) = default;

	/**
	 * construct, no rotation, no translation
	 */
	inline Transform() :
			rx(0), ry(0), rz(0), rw(1), tx(0), ty(0), tz(0) {
	}

	/**
	 * construct from rotation and translation
	 *
	 * @param t translation
	 * @param r rotation
	 */
	inline Transform(const Vec3 & t, const Quat & r) :
					rx(r.getX()),
					ry(r.getY()),
					rz(r.getZ()),
					rw(r.getW()),
					tx(t.getX()),
					ty(t.getY()),
					tz(t.getZ()) {
	}

	/**
	 * default destructor
	 */
	inline virtual ~Transform() = default;

	/**
	 * transform as mat4
	 *
	 * @return mat4 of transform
	 */
	Mat4 asMat4() const;

	/**
	 * calculate distance from this transform to other transform
	 *
	 * @param other transform to calculate distance to
	 *
	 * @return distance from this to other
	 */
	double distanceTo(const Transform & other) const;

	/**
	 * get inverse of transform rotation in matrix form
	 *
	 * @return inverse of transform rotation
	 */
	Mat3 getInverseRotationMatrix() const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	std::shared_ptr<ScriptObject> getMember(ScriptExecutionState & execState,
			const std::string & name) const;

	/**
	 * get transform rotation
	 *
	 * @return transform rotation
	 */
	inline Quat getRotation() const {
		return Quat(rx, ry, rz, rw);
	}

	/**
	 * get transform rotation as matrix
	 *
	 * @return transform rotation
	 */
	Mat3 getRotationMatrix() const;

	/**
	 * get transform translation
	 *
	 * @return transform translation
	 */
	inline Vec3 getTranslation() const {
		return Vec3(tx, ty, tz);
	}

	/**
	 * interpolate between 'this' and 'other', replacing 'this'
	 *
	 * t = 0, 'this' unchanged
	 * t = 1, 'this' = other
	 *
	 * T = (1-t) * T + t * T'
	 * R = (1-t) * R + t * R'
	 *
	 * @param other
	 * @param t
	 */
	void interpolate(const Transform & other, float t);

	/**
	 * calculate inverse of transform
	 *
	 * @return inverse of transform
	 */
	Transform inverse() const;

	/**
	 * Transform point by the inverse of this transform, replacing point with
	 * result
	 *
	 * @param point
	 *            point to transform
	 */
	void inverseTransformPoint(Vec3 & point) const;

	inline bool operator!=(const Transform & other) const {
		return !(*this == other);
	}

	/**
	 * this equals other
	 *
	 * @param other transform to compare against
	 *
	 * @return true if this equals other, false otherwise
	 */
	inline bool operator==(const Transform & other) const {
		return rx == other.rx && ry == other.ry && rz == other.rz
				&& rw == other.rw && tx == other.tx && ty == other.ty
				&& tz == other.tz;
	}

	/**
	 * Transform normal by this transform, replacing normal with result
	 *
	 * @param normal
	 *            point to transform
	 */
	void rotate(Normal & normal) const;

	/**
	 * Rotate by quaternion, and normalize
	 *
	 * T = T
	 * R = R * r
	 *
	 * @param r
	 * 			rotation to apply
	 */
	void rotate(const Quat & r);

	/**
	 * Transform normal by the inverse of this transform, replacing normal with
	 * result
	 *
	 * @param normal
	 *            point to transform
	 */
	void rotateInverse(Normal & normal) const;

	/**
	 * set this transform equal to other
	 *
	 * @param other transform to copy
	 */
	inline void set(const Transform & other) {
		rx = other.rx;
		ry = other.ry;
		rz = other.rz;
		rw = other.rw;
		tx = other.tx;
		ty = other.ty;
		tz = other.tz;
	}

	/**
	 * Set rotation to angle about axis
	 *
	 * @param axis
	 *            rotation axis
	 * @param angle
	 *            rotation angle
	 */
	void setRotation(const Vec3 & axis, float angle);

	/**
	 * set the rotation of this transform
	 *
	 * @param r rotation to copy
	 */
	inline void setRotation(const Quat & r) {
		rx = r.getX();
		ry = r.getY();
		rz = r.getZ();
		rw = r.getW();
	}

	/**
	 * set the translation of this transform
	 *
	 * @param t translation to copy
	 */
	inline void setTranslation(const Vec3 & t) {
		tx = t.getX();
		ty = t.getY();
		tz = t.getZ();
	}

	/**
	 * Calculate the transform that will take an object from 'this' space to
	 * 'other' space
	 *
	 * @return transform that takes objects from 'this' space into 'other' space
	 *         inverse( 'other' ) * 'this'
	 */
	Transform to(const Transform & other) const;

	/**
	 *
	 *
	 * @return transform as string
	 */
	std::string toString() const;

	/**
	 * transform 'other' by 'this', replacing 'this'
	 *
	 * T = T + T * R * T'
	 * R = R * R'
	 *
	 * @param other
	 */
	void transform(const Transform & other);

	/**
	 * replace point with transformed point
	 *
	 * p = T * R * p
	 *
	 * @param point
	 */
	void transformPoint(Vec3 & point) const;

	/**
	 * Add translation to transform
	 *
	 * T = T + inv(R) * t
	 * R = R
	 *
	 * @param t
	 *            translation to be applied
	 */
	void translate(const Vec3 & t);

	/**
	 * get script object factory for Transform
	 *
	 * @return  Transform factory
	 */
	static ScriptObjectPtr getFactory();

private:
	float rx;
	float ry;
	float rz;
	float rw;
	double tx;
	double ty;
	double tz;
};

