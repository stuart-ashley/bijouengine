#pragma once

#include "../scripting/scriptObject.h"

class BoundingBox;
class ConvexHull;
class IndexArray;
class Intersection;
class Sphere;
class Transform;
class TriangleList;
class Vec3Array;

class Terrain: public ScriptObject {
public:

	/**
	 * constructor
	 *
	 * @param gridSize
	 * @param vertices
	 * @param indices
	 */
	Terrain(float gridSize, const Vec3Array & vertices,
			const IndexArray & indices);

	/**
	 * destructor
	 */
	~Terrain();

	/**
	 * Collide terrain with bounding box
	 *
	 * @param box           bounding box to intersect against
	 * @param boxToTerrain  from box space to terrain space
	 * @param intersection  intersection, written only if collision
	 *
	 * @return              true if collision, false otherwise
	 */
	bool collide(const BoundingBox & box, const Transform & boxToTerrain,
			Intersection & intersection);

	/**
	 * Collide terrain with convex hull
	 *
	 * @param hull           other body to intersect against
	 * @param hullToTerrain  from hull space to terrain space
	 * @param intersection   intersection, written only if collision
	 *
	 * @return               true if collision, false otherwise
	 */
	bool collide(const ConvexHull & hull, const Transform & hullToTerrain,
			Intersection & intersection);

	/**
	 * Collide terrain against sphere
	 *
	 * @param sphere        sphere to collide against
	 * @param sphereToThis  from sphere space to terrain space
	 * @param intersection  intersection, written only if collision
	 *
	 * @return              true if any part of sphere under terrain, false otherwise
	 */
	bool collide(const Sphere & sphere, const Transform & sphereToThis,
			Intersection & intersection);

	/**
	 *
	 * @return
	 */
	const BoundingBox & getBounds() const;

	/**
	 *
	 * @return
	 */
	const TriangleList & getTriangleList() const;

	/**
	 * Validate vertices and indices, once valid create bounds and list of
	 * triangles at which point vertices and indices become redundant
	 *
	 * @return  true if vertices and indices valid, false otherwise
	 */
	bool validate();

	/**
	 * get script object factory for Terrain
	 *
	 * @return  Terrain factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

