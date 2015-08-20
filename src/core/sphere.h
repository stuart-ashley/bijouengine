#pragma once

#include "../scripting/scriptObject.h"

class BoundingBox;
class Intersection;
class Ray;
class Transform;
class TriangleList;
class Vec3;

class Sphere: public ScriptObject {
public:

	/**
	 * construct sphere from point and radius
	 *
	 * @param point   centre of sphere
	 * @param radius  radius of sphere
	 */
	Sphere(const Vec3 & point, float radius);

	/**
	 * destructor
	 */
	~Sphere();

	/**
	 * Collide sphere against sphere, update intersection if collision
	 *
	 * @param other         sphere to collide against
	 * @param otherToThis   transform from other sphere space to this sphere space
	 * @param intersection  intersection, updated if collision occurs
	 *
	 * @return              true if spheres collide, false otherwise
	 */
	bool collide(const Sphere & other, const Transform & otherToThis,
			Intersection & intersection) const;

	/**
	 * Collide sphere against bounding box, update intersection if collision
	 *
	 * @param box           bounding box to collide against
	 * @param boxToThis     transform from box space to this sphere space
	 * @param intersection  intersection, updated if collision occurs
	 *
	 * @return              true if collision, false otherwise
	 */
	bool collide(const BoundingBox & box, const Transform & boxToThis,
			Intersection & intersection) const;

	/**
	 * get bounds of sphere
	 *
	 * @return  bounds
	 */
	const BoundingBox & getBounds() const;

	/**
	 * get centre of sphere
	 *
	 * @return  centre
	 */
	const Vec3 & getPoint() const;

	/**
	 * get sphere radius
	 *
	 * @return  radius
	 */
	float getRadius() const;

	/**
	 * sphere as icosahedron
	 *
	 * @return  icosahedron triangles
	 */
	const TriangleList & getTriangleList() const;

	/**
	 * Intersection against ray. Negative ray intersections are discarded. If
	 * ray starts outside sphere, intersection is at entry. If ray starts in
	 * sphere, intersection is at exit.
	 *
	 * @param ray       ray to insect with
	 * @param point     written if intersection occurs, intersection point on
	 *                  surface of sphere
	 * @param distance  written if intersection occurs, distance along ray to
	 *                  intersection point on the surface of the sphere
	 *
	 * @return result of intersection
	 */
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	/**
	 * get script object factory for Sphere
	 *
	 * @return  Sphere factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

