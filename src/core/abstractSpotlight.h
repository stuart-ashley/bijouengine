#pragma once

#include "abstractLight.h"
#include "convexHull.h"

#include <cmath>

class AbstractSpotlight: public AbstractLight {
public:

	/**
	 * construct spotlight
	 *
	 * @param name      name of spotlight
	 * @param color     color of spotlight
	 * @param distance  distance of spotlight
	 * @param angle     angle of spotlight
	 * @param blend     amount of blend
	 */
	AbstractSpotlight(const std::string & name, const Color & color,
			float distance, float angle, float blend);

	/**
	 * get spotlight angle
	 *
	 * @return  spotlight angle
	 */
	float getAngle() const;

	/**
	 * get spotlight convex hull
	 *
	 * @return  convex hull of spotlight
	 */
	const ConvexHull & getConvexHull() const;

	/**
	 * get spotlight distance
	 *
	 * @return  spotlight distance
	 */
	float getDistance() const;

	/**
	 * get half angle in radians
	 *
	 * @return  half angle in radians
	 */
	float getHalfAngleRadians() const;

private:
	float distance;
	float halfRadians;
	float blend;
	ConvexHull convexHull;
};

