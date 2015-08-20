#pragma once

#include "normal.h"
#include "vec3.h"

#include "../scripting/scriptObject.h"

class Quat final: public ScriptObject {
public:

	/**
	 * construct quaternion from pitch & yaw
	 *
	 * @param pitch
	 * @param yaw
	 */
	Quat(float pitch, float yaw);

	/**
	 * construct quaternion from axis and angle
	 *
	 * @param axis   axis for quaternion
	 * @param angle  angle for quaternion
	 */
	Quat(const Normal & axis, float angle);

	/**
	 * default destructor
	 */
	inline virtual ~Quat() = default;

	inline Quat() :
			m_x(0), m_y(0), m_z(0), m_w(0) {
	}

	inline Quat(float x, float y, float z, float w) :
			m_x(x), m_y(y), m_z(z), m_w(w) {
	}

	inline Quat conjugate() const {
		return Quat(-m_x, -m_y, -m_z, m_w);
	}

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	inline float getW() const {
		return m_w;
	}

	inline float getX() const {
		return m_x;
	}

	inline float getY() const {
		return m_y;
	}

	inline float getZ() const {
		return m_z;
	}

	void interpolate(const Quat & q, const Quat & r, float t);

	inline void interpolate(const Quat & q, float t) {
		interpolate(*this, q, t);
	}

	/**
	 * quaternion multiple
	 *
	 * @param r  right hand quaternion
	 *
	 * @return   this multiplied by r
	 */
	Quat operator*(const Quat & r) const;

	/**
	 * in place quaternion multiple
	 *
	 * @param other  quaternion
	 *
	 * @return       reference to this
	 */
	Quat & operator*=(const Quat & r);

	/**
	 * rotate normal by quaternion
	 *
	 * n' = q * n * conj(q)
	 *
	 * @param n  normal to rotate
	 *
	 * @return   post rotation normal
	 */
	Normal rotate(const Normal & n) const;

	/**
	 * rotate vector by quaternion
	 *
	 * v' = q * v * conj(q)
	 *
	 * @param v  vector to rotate
	 *
	 * @return   post rotation vector
	 */
	Vec3 rotate(const Vec3 & v) const;

	/**
	 * rotate Vec3( sx, 0, 0 ) by this rotation
	 */
	Vec3 rotateX(double sx);

	/**
	 * rotate Vec3( 0, sy, 0 ) by this rotation
	 */
	Vec3 rotateY(double sy);

	/**
	 * rotate Vec3( 0, 0, sz ) by this rotation
	 */
	Vec3 rotateZ(double sz);

	/**
	 * set quaternion from pitch & yaw
	 *
	 * @param pitch
	 * @param yaw
	 */
	void set(float pitch, float yaw);

	/**
	 * set quaternion and normalize
	 */
	void set(float x, float y, float z, float w);

	inline std::string toString() const override {
		return "Quat( " + std::to_string(m_x) + ", " + std::to_string(m_y)
				+ ", " + std::to_string(m_z) + ", " + std::to_string(m_w) + " )";
	}

	/**
	 * get script object factory for Quat
	 *
	 * @return  Quat factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	float m_x;
	float m_y;
	float m_z;
	float m_w;
};
