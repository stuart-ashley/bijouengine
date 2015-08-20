#pragma once

#include <memory>
#include <string>

class Binary;

/**
 * Cubic bezier B(t) = (1-t)³ P₀ + 3 (1-t)² t P₁ + 3 (1-t) t² P₂ + t³ P₃, * t∈[0,1]
 */
class Bezier {
public:

	/**
	 * constructor
	 *
	 * @param binary
	 */
	Bezier(const Binary & binary);

	/**
	 * get name of bezier curve
	 *
	 * @return  name of curve
	 */
	const std::string & getName() const;

	/**
	 * get y for given x
	 *
	 * @param x  x value to find y for
	 *
	 * @return   y value for given x
	 */
	float getY(float x) const;

	/**
	 * validate curve (ie check loaded)
	 *
	 * @return  true if valid, false otherwise
	 */
	bool validate() const;

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

