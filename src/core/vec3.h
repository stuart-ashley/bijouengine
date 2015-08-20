#pragma once

#include "normal.h"

#include "../scripting/scriptObject.h"

#include <string>

class Vec3: public ScriptObject {
public:

	Vec3(double x, double y, double z);

	inline Vec3() :
			m_x(0), m_y(0), m_z(0) {
	}

	inline Vec3(const Vec3 & v) :
			m_x(v.m_x), m_y(v.m_y), m_z(v.m_z) {
	}

	inline ~Vec3() {
	}

	inline Vec3 cross(const Normal & n) const {
		return Vec3(m_y * n.getZ() - m_z * n.getY(),
				m_z * n.getX() - m_x * n.getZ(),
				m_x * n.getY() - m_y * n.getX());
	}

	inline Vec3 cross(const Vec3 & other) const {
		return Vec3(m_y * other.m_z - m_z * other.m_y,
				m_z * other.m_x - m_x * other.m_z,
				m_x * other.m_y - m_y * other.m_x);
	}

	inline void cross(const Normal & n, const Vec3 & v) {
		m_x = n.getY() * v.m_z - n.getZ() * v.m_y;
		m_y = n.getZ() * v.m_x - n.getX() * v.m_z;
		m_z = n.getX() * v.m_y - n.getY() * v.m_x;
	}

	inline void cross(const Vec3 & u, const Vec3 & v) {
		*this = u.cross(v);
	}

	inline double dot(const Normal & normal) const {
		return m_x * normal.getX() + m_y * normal.getY() + m_z * normal.getZ();
	}

	inline double dot(const Vec3 & other) const {
		return m_x * other.m_x + m_y * other.m_y + m_z * other.m_z;
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

	inline double getX() const {
		return m_x;
	}

	inline double getY() const {
		return m_y;
	}

	inline double getZ() const {
		return m_z;
	}

	/**
	 * this = u * (1 - t) + v * t
	 *
	 * @param u
	 * @param v
	 * @param t
	 */
	void interpolate(const Vec3 & u, const Vec3 & v, double t);

	double length() const;

	inline Normal normalized() const {
		return Normal(m_x, m_y, m_z);
	}

	inline Vec3 operator*(double scale) const {
		return Vec3(m_x * scale, m_y * scale, m_z * scale);
	}

	inline Vec3 & operator*=(double scale) {
		m_x *= scale;
		m_y *= scale;
		m_z *= scale;
		return *this;
	}

	inline Vec3 operator+(const Vec3 & other) const {
		return Vec3(m_x + other.m_x, m_y + other.m_y, m_z + other.m_z);
	}

	inline Vec3 & operator+=(Vec3 const & v) {
		m_x += v.m_x;
		m_y += v.m_y;
		m_z += v.m_z;
		return *this;
	}

	inline Vec3 operator-() const {
		return Vec3(-m_x, -m_y, -m_z);
	}

	inline Vec3 operator-(const Vec3 & other) const {
		return Vec3(m_x - other.m_x, m_y - other.m_y, m_z - other.m_z);
	}

	inline Vec3 operator/(double f) const {
		return Vec3(m_x / f, m_y / f, m_z / f);
	}

	inline Vec3 & operator/=(double f) {
		m_x /= f;
		m_y /= f;
		m_z /= f;
		return *this;
	}

	inline bool operator==(const Vec3 & other) const {
		return m_x == other.m_x && m_y && other.m_y && m_z == other.m_z;
	}

	void scale(double s, const Normal & n);

	void scaleAdd(double s, const Normal & n, const Vec3 & v);

	void scaleAdd(double s, const Vec3 & u, const Vec3 & v);

	void scaleAdd(double s, const Vec3 & v);

	inline void set(double x, double y, double z) {
		m_x = x;
		m_y = y;
		m_z = z;
	}

	/**
	 * set named script object member
	 *
	 * @param name   name of member
	 * @param value  desired value
	 */
	void setMember(const std::string & name, const ScriptObjectPtr & value)
			override;

	inline void setX(double x) {
		m_x = x;
	}

	inline void setY(double y) {
		m_y = y;
	}

	inline void setZ(double z) {
		m_z = z;
	}

	inline std::string toString() const override {
		return "Vec3( " + std::to_string(m_x) + ", " + std::to_string(m_y) + ","
				+ std::to_string(m_z) + " )";
	}

	/**
	 * get script object factory for Vec3
	 *
	 * @return  Vec3 factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	double m_x;
	double m_y;
	double m_z;
};
