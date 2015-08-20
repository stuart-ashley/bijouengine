#pragma once

#include "../scripting/scriptObject.h"

#include <string>

class Vec2 final: public ScriptObject {
public:

	inline Vec2() :
			m_x(0), m_y(0) {
	}

	inline Vec2(float x, float y) :
			m_x(x), m_y(y) {
	}

	/**
	 * default destructor
	 */
	inline virtual ~Vec2() = default;

	inline float dot(const Vec2 & other) const {
		return m_x * other.m_x + m_y * other.m_y;
	}

	void dump() const;

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

	inline float getX() const {
		return m_x;
	}

	inline float getY() const {
		return m_y;
	}

	float length() const;

	inline Vec2 operator *(float f) const {
		return Vec2(m_x * f, m_y * f);
	}

	inline Vec2 operator +(const Vec2 & other) const {
		return Vec2(m_x + other.m_x, m_y + other.m_y);
	}

	inline Vec2 operator -(const Vec2 & other) const {
		return Vec2(m_x - other.m_x, m_y - other.m_y);
	}

	inline std::string toString() const override {
		return "Vec2( " + std::to_string(m_x) + ", " + std::to_string(m_y)
				+ " )";
	}

	/**
	 * get script object factory for Vec2
	 *
	 * @return  Vec2 factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	float m_x;
	float m_y;
};
