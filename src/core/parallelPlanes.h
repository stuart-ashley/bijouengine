#pragma once

#include <memory>

class BoundingBox;
class ConvexHull;
class IndexArray;
class Intersection;
class Ray;
class Transform;
class TriangleList;
class Vec3;
class Vec3Array;

class ParallelPlanes {
public:

	/**
	 *
	 * @param vertices
	 * @param indices
	 */
	ParallelPlanes(const Vec3Array & vertices, const IndexArray & indices);

	/**
	 * Collide ParallelPlanes with convex hull
	 *
	 * @param hull                  other body to intersect against
	 * @param hullToParallelPlanes
	 * @param intersection          intersection, written only if collision
	 *
	 * @return                      true if collision, false otherwise
	 */
	bool collide(const ConvexHull & hull,
			const Transform & hullToParallelPlanes,
			Intersection & intersection) const;

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
	 * Intersect against ray, write point and distance and return true if
	 * intersection occurs, otherwise return false leaving point and distance
	 * untouched
	 *
	 * @param ray       ray to intersect against
	 * @param point     written if intersection, point of intersection
	 * @param distance  written if intersection, distance from ray start to intersection
	 *
	 * @return          has intersection occurred
	 */
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	/**
	 * Validate vertices and indices, once valid create bounds and list of
	 * triangles at which point vertices and indices become redundant
	 *
	 * @return  true if vertices and indices valid, false otherwise
	 */
	bool validate();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

