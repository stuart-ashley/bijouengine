#pragma once

#include "../scripting/scriptObject.h"

class ScriptExecutionState;

class Color final: public ScriptObject {
public:

	Color(const Color &) = default;

	inline Color(float r, float g, float b) :
			r(r), g(g), b(b), a(1) {
	}

	inline Color(float r, float g, float b, float a) :
			r(r), g(g), b(b), a(a) {
	}

	/**
	 * default destructor
	 */
	inline virtual ~Color() = default;

	inline float getA() const {
		return a;
	}

	inline float getB() const {
		return b;
	}

	inline float getG() const {
		return g;
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

	inline float getR() const {
		return r;
	}

	inline Color operator*(float s) const {
		return Color(r * s, g * s, b * s, a * s);
	}

	inline Color operator+(const Color & other) const {
		return Color(r + other.r, g + other.g, b + other.b, a + other.a);
	}

	inline Color operator-(const Color & other) const {
		return Color(r - other.r, g - other.g, b - other.b, a - other.a);
	}

	inline Color operator/(float s) const {
		return Color(r / s, g / s, b / s, a / s);
	}

	inline bool operator==(const Color & other) const {
		return r == other.r && g == other.g && b == other.b && a == other.a;
	}

	std::string toString() const override;

	static Color black();

	static Color blue();

	/**
	 * get script object factory for Color
	 *
	 * @return  Color factory
	 */
	static const ScriptObjectPtr & getFactory();

	static Color green();

	static Color red();

	static Color white();

private:
	float r;
	float g;
	float b;
	float a;
};

