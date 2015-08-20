#pragma once

#include "../scripting/scriptObject.h"

class Normal final: public ScriptObject {
public:
	inline Normal(const Normal &) = default;

	Normal(double x, double y, double z);

	Normal(float x, float y, float z);

	/**
	 * default destructor
	 */
	inline virtual ~Normal() = default;

	inline Normal cross(const Normal & other) const {
		float x = m_y * other.m_z - m_z * other.m_y;
		float y = m_z * other.m_x - m_x * other.m_z;
		float z = m_x * other.m_y - m_y * other.m_x;
		return Normal(x, y, z);
	}

	inline float dot(const Normal & other) const {
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

	inline float getX() const {
		return m_x;
	}

	inline float getY() const {
		return m_y;
	}

	inline float getZ() const {
		return m_z;
	}

	inline bool invalid() const {
		return m_x == 0 && m_y == 0 && m_z == 0;
	}

	inline Normal operator-() const {
		return Normal(-m_x, -m_y, -m_z);
	}

	inline bool operator==(const Normal & other) const {
		return m_x == other.m_x && m_y == other.m_y && m_z == other.m_z;
	}

	void set(float x, float y, float z);

	inline std::string toString() const override {
		return "Normal(" + std::to_string(m_x) + ", " + std::to_string(m_y)
				+ ", " + std::to_string(m_z) + ")";
	}

	/**
	 * get script object factory for Normal
	 *
	 * @return  Normal factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	float m_x;
	float m_y;
	float m_z;
};

