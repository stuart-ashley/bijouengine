#pragma once

#include "vec3.h"

#include "../scripting/scriptObject.h"

#include <memory>

class Intersection;
class Ray;
class Transform;

class BoundingBox final: public ScriptObject {
public:
	/**
	 * construct bounding box from single point
	 *
	 * @param a  first point
	 */
	BoundingBox(const Vec3 & a);

	/**
	 * construct bounding box from two points
	 *
	 * @param a  first point
	 * @param b  second point
	 */
	BoundingBox(const Vec3 & a, const Vec3 & b);

	/**
	 * construct bounding box from three points
	 *
	 * @param a  first point
	 * @param b  second point
	 * @param c  third point
	 */
	BoundingBox(const Vec3 & a, const Vec3 & b, const Vec3 & c);

	/**
	 * copy constructor
	 *
	 * @param  other bounds
	 */
	BoundingBox(const BoundingBox &) = default;

	/**
	 * default destructor
	 */
	inline virtual ~BoundingBox() = default;

	/**
	 * Collide two boxes
	 *
	 * @param that          box to collide against
	 * @param thatToThis    from 'that' space to 'this' space
	 * @param intersection  intersection, written only if collision
	 *
	 * @return              true if collision, false otherwise
	 */
	bool collide(const BoundingBox & that, const Transform & thatToThis,
			Intersection & intersection) const;

	/**
	 * does this bounds contain the other bounds
	 * ie would this bounds union other bounds equal this bounds
	 *
	 * @param other  other bounds
	 *
	 * @return       true if other bounds in contained within this bounds,
	 *               false otherwise
	 */
	bool contains(const BoundingBox & other) const;

	/**
	 * calculate centre of bounds
	 *
	 * @return  centre of bounds
	 */
	Vec3 getCentre() const;

	/**
	 * minimum and maximum point in given direction
	 *
	 * @param direction  direction to get values for
	 * @param minPoint   minimum point in direction
	 * @param maxPoint   maximum point in direction
	 */
	void getDirectionMinMax(const Normal & direction, Vec3 & minPoint,
			Vec3 & maxPoint) const;

	/**
	 * calculate extents of bounds
	 * ie width, height, depth
	 *
	 * @return  extents of bounds
	 */
	Vec3 getExtents() const;

	/**
	 * get maximum point of bounds
	 *
	 * @return  maximum point of bounds
	 */
	const Vec3 & getMax() const;

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

	/**
	 * get minimum point of bounds
	 *
	 * @return  minimum point of bounds
	 */
	const Vec3 & getMin() const;

	/**
	 * calculate radius of bounds
	 *
	 * @return  radius of bounds
	 */
	double getRadius() const;

	/**
	 * calculate surface area of bounds
	 *
	 * @return  surface area of bounds
	 */
	double getSurfaceArea() const;

	/**
	 * calculate volume of bounds
	 *
	 * @return  volume of bounds
	 */
	double getVolume() const;

	/**
	 * does this bounds intersect other bounds
	 * ie does this bounds overlap other bounds
	 *    it is not enough to share a face, edge or vertex
	 *
	 * @param other  bounds test against
	 *
	 * @return       true if this bounds intersects other bounds
	 *               false otherwise
	 */
	bool intersects(const BoundingBox & other) const;

	/**
	 * is bounding box empty
	 *
	 * @return  true if empty bounds, false otherwise
	 */
	bool isEmpty() const;

	/**
	 * union of this this and other bounds
	 *
	 * @param other  other bounds
	 *
	 * @return       union of bounds
	 */
	BoundingBox operator+(const BoundingBox & other) const;

	/**
	 * add other bounds to this bounds
	 *
	 * @param other  bounds to form union with
	 */
	BoundingBox & operator+=(const BoundingBox & other);

	/**
	 * add point to this bounds
	 *
	 * @param point  point to form union with
	 */
	BoundingBox & operator+=(const Vec3 & point);

	/**
	 * Intersection against ray, tmin & tmax are the extents of the ray
	 * and are truncated after intersection, if there is no intersection
	 * this may possibly create a negative extents ie tmin > tmax
	 *
	 * @param ray   ray to insect with
	 * @param tmin  minimum extent of ray
	 * @param tmax  maximum extent of ray
	 *
	 * @return      result of intersection
	 */
	bool rayIntersect(const Ray & ray, double & tmin, double & tmax) const;

	/**
	 * Intersection against ray. Negative ray intersections are discarded. If
	 * ray starts outside box, intersection is at entry. If ray starts in box,
	 * intersection is at exit.
	 *
	 * @param ray       ray to insect with
	 * @param point     written if intersection occurs, returns intersection point on
	 *                  surface of bounding box
	 * @param distance  written if intersection occurs, returns distance along ray to
	 *                  intersection point on the surface on the bounding box
	 *
	 * @return          result of intersection
	 */
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	/**
	 * calculate string representation of bounds
	 *
	 * @return  bounds as string
	 */
	std::string toString() const override;

	/**
	 * transform bounding box
	 *
	 * @param transform  transform for bounding box
	 *
	 * @return           transformed bounding box
	 */
	BoundingBox transformed(const Transform & transform) const;

	/**
	 * get empty bounding box
	 *
	 * @return  empty bounding box
	 */
	static const BoundingBox & empty();

	/**
	 * get script object factory for BoundingBox
	 *
	 * @return  BoundingBox factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	/**
	 * construct empty bounding box
	 */
	BoundingBox();

	/**
	 * Intersection against ray in x-axis only, tmin & tmax are the extents
	 * of the ray and are truncated after intersection, if there is no
	 * intersection this may possibly create a negative extents ie tmin > tmax
	 *
	 * @param ray   ray to insect with
	 * @param tmin  minimum extent of ray
	 * @param tmax  maximum extent of ray
	 *
	 * @return      result of intersection
	 */
	bool rayIntersectX(const Ray & ray, double & tmin, double & tmax) const;

	/**
	 * Intersection against ray in y-axis only, tmin & tmax are the extents
	 * of the ray and are truncated after intersection, if there is no
	 * intersection this may possibly create a negative extents ie tmin > tmax
	 *
	 * @param ray   ray to insect with
	 * @param tmin  minimum extent of ray
	 * @param tmax  maximum extent of ray
	 *
	 * @return      result of intersection
	 */
	bool rayIntersectY(const Ray & ray, double & tmin, double & tmax) const;

	/**
	 * Intersection against ray in z-axis only, tmin & tmax are the extents
	 * of the ray and are truncated after intersection, if there is no
	 * intersection this may possibly create a negative extents ie tmin > tmax
	 *
	 * @param ray   ray to insect with
	 * @param tmin  minimum extent of ray
	 * @param tmax  maximum extent of ray
	 *
	 * @return      result of intersection
	 */
	bool rayIntersectZ(const Ray & ray, double & tmin, double & tmax) const;

	Vec3 m_min;
	Vec3 m_max;
};

