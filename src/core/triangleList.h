#pragma once

#include "triangle.h"

#include <vector>

class ConvexHull;
class IndexArray;
class Vec3Array;

class TriangleList {
public:

	/**
	 * empty constructor
	 */
	TriangleList();

	/**
	 * constructor from vector
	 *
	 * @param triangles
	 */
	TriangleList(const std::vector<Triangle> & triangles);

	/**
	 * construct from vertices & indices
	 *
	 * @param indices
	 * @param vertices
	 */
	TriangleList(const IndexArray & indices, const Vec3Array & vertices);

	inline void add(const Triangle & triangle) {
		triangles.emplace_back(triangle);
	}

	inline std::vector<Triangle>::const_iterator begin() const {
		return triangles.cbegin();
	}

	inline std::vector<Triangle>::const_iterator end() const {
		return triangles.cend();
	}

	inline const Triangle & get(size_t index) const {
		return triangles.at(index);
	}

	/**
	* Calculate intersection of triangle list against bounding box
	*
	* @param box           bounding box to intersect against
	* @param intersection  intersection, written only if intersection
	*
	* @return              true if intersection, false otherwise
	*/
	bool intersection(const BoundingBox & box,
			Intersection & intersection) const;

	/**
	* Calculate intersection of triangle list against convex hull
	*
	* @param hull          convex hull to intersect against
	* @param intersection  intersection, written only if intersection
	*
	* @return              true if intersection, false otherwise
	*/
	bool intersection(const ConvexHull & hull,
			Intersection & intersection) const;

	/**
	 * Intersect against ray, write point and distance and return true if
	 * intersection occurs, otherwise return false leaving point and distance
	 * untouched
	 *
	 * Distance is negative if ray start inside triangle list
	 *
	 * Find all intersections alone ray in positive and negative directions. If
	 * there is an odd number of intersections in the negative direction the ray
	 * start must be inside the shape and the nearest negative intersection to
	 * the ray start is returned. If there is a even or zero number of
	 * intersections in the negative direction the ray start must be outside the
	 * shape and the nearest positive intersection is returned
	 *
	 * triangle list needs to be closed
	 *
	 * @param ray       ray to intersect against
	 * @param point     written if intersection, point of intersection
	 * @param distance  written if intersection, distance from ray start to
	 *                  intersection
	 *
	 * @return          has intersection occurred
	 */
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	inline size_t size() const {
		return triangles.size();
	}

private:
	std::vector<Triangle> triangles;
};

