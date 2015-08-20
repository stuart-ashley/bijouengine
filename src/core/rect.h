#pragma once

#include "../scripting/scriptObject.h"

class Vec2;

class Rect: public ScriptObject {
public:

	Rect(const Rect &) = default;

	/**
	 * constructor
	 *
	 * @param left
	 * @param right
	 * @param bottom
	 * @param top
	 */
	Rect(float left, float right, float bottom, float top);

	/**
	 * destructor
	 */
	~Rect();

	bool contains(const Vec2 & point) const;

	inline float getAspectRatio() const {
		return getWidth() / getHeight();
	}

	inline float getBottom() const {
		return m_bottom;
	}

	inline float getHeight() const {
		return m_top - m_bottom;
	}

	inline float getLeft() const {
		return m_left;
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

	inline float getRight() const {
		return m_right;
	}

	inline float getTop() const {
		return m_top;
	}

	inline float getWidth() const {
		return m_right - m_left;
	}

	inline bool intersects(const Rect & other) const {
		return m_left <= other.m_right && m_right >= other.m_left
				&& m_bottom <= other.m_top && m_top >= other.m_bottom;
	}

	void set(float left, float right, float bottom, float top);

	inline void set(const Rect & other) {
		m_left = other.m_left;
		m_right = other.m_right;
		m_bottom = other.m_bottom;
		m_top = other.m_top;
	}

	void setMember(const std::string & name, const ScriptObjectPtr & value)
			override;

	std::string toString() const override;

	/**
	 * get script object factory for Rect
	 *
	 * @return  Rect factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	float m_left;
	float m_right;
	float m_bottom;
	float m_top;
};

