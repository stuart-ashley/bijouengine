#pragma once

#include "../scripting/scriptObject.h"

class BoundingBox;
class ConvexHull;
class DebugGeometry;
class Intersection;
class ParallelPlanes;
class Ray;
class Sphere;
class Terrain;
class Transform;
class Vec3;

#include <vector>

class CollisionHierarchy: public ScriptObject {
public:

	/**
	 * Construct from bounding box and children
	 *
	 * @param bounds
	 * @param children
	 */
	CollisionHierarchy(const std::shared_ptr<BoundingBox> & bounds,
			const std::vector<CollisionHierarchy> & children);

	/**
	 * Construct from sphere and children
	 *
	 * @param bounds
	 * @param children
	 */
	CollisionHierarchy(const std::shared_ptr<Sphere> & sphere,
			std::vector<CollisionHierarchy> & children);

	/**
	 * Construct from convex hull and children
	 *
	 * @param hull
	 * @param children
	 */
	CollisionHierarchy(const std::shared_ptr<ConvexHull> & hull,
			std::vector<CollisionHierarchy> & children);

	/**
	 * Construct from terrain mesh and children
	 *
	 * @param terrain
	 * @param children
	 */
	CollisionHierarchy(const std::shared_ptr<Terrain> & terrain,
			std::vector<CollisionHierarchy> & children);

	/**
	 * Construct from parallel planes mesh and children
	 *
	 * @param terrain
	 * @param children
	 */
	CollisionHierarchy(const std::shared_ptr<ParallelPlanes> & planes,
			std::vector<CollisionHierarchy> & children);

	/**
	 * destructor
	 */
	~CollisionHierarchy();

	/**
	 * Collide two hierarchies, traversing 'this' children
	 *
	 * @param that
	 * @param toThis
	 * @return
	 */
	std::vector<Intersection> collide(const CollisionHierarchy & that,
			const Transform & toThis) const;

	/**
	 * Draw debug geometry to list
	 *
	 * @param debug
	 * @param toWorld
	 */
	void draw(std::vector<DebugGeometry> & debug,
			const Transform & toWorld) const;

	/**
	 * get bounds
	 *
	 * @return bounding box of collision hierarchy
	 */
	const BoundingBox & getBounds() const;

	/**
	 * ray intersection
	 *
	 * @param ray
	 * @param point
	 * @param distance
	 * @return
	 */
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	/**
	 * validate
	 *
	 * @return true, if valid, otherwise false
	 */
	bool validate() const;

	/**
	 * get script object factory for CollisionHierarchy
	 *
	 * @return  CollisionHierarchy factory
	 */
	static ScriptObjectPtr getFactory();

private:
	/**
	 * Leaf node collision
	 *
	 * @param that
	 * @param toThis
	 * @return
	 */
	std::vector<Intersection> leafCollide(const CollisionHierarchy & that,
			const Transform & toThis) const;

	/**
	 * Collide two hierarchies, traversing 'that' children
	 *
	 * @param that
	 * @param toThis
	 * @return
	 */
	std::vector<Intersection> rCollide(const CollisionHierarchy & that,
			const Transform & toThis) const;

	struct impl;
	std::shared_ptr<impl> pimpl;
};

