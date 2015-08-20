#pragma once

#include "plane.h"

#include "../scripting/scriptObject.h"

#include <memory>
#include <vector>

class BoundingBox;
class IndexArray;
class Intersection;
class Mat4;
class NormalArray;
class Ray;
class Sphere;
class Transform;
class TriangleList;
class Vec3;
class Vec3Array;

class ConvexHull: public ScriptObject {
public:

	class Edge {
	public:
		Edge(const Edge &) = default;

		inline Edge(size_t v0, size_t v1, size_t f0, size_t f1) {
			vertexIdx[0] = v0;
			vertexIdx[1] = v1;
			faceNormalIdx[0] = f0;
			faceNormalIdx[1] = f1;
		}

		inline size_t getFaceNormalIndex0() const {
			return faceNormalIdx[0];
		}

		inline size_t getFaceNormalIndex1() const {
			return faceNormalIdx[1];
		}

		inline size_t getVertexIndex0() const {
			return vertexIdx[0];
		}

		inline size_t getVertexIndex1() const {
			return vertexIdx[1];
		}

		inline void setVertexIndex0(size_t index) {
			vertexIdx[0] = index;
		}

		inline void setVertexIndex1(size_t index) {
			vertexIdx[1] = index;
		}
	private:
		size_t vertexIdx[2];
		size_t faceNormalIdx[2];
	};

	/**
	 * Construct from bounding box
	 */
	explicit ConvexHull(const BoundingBox & box);

	/**
	 * Constructor
	 */
	ConvexHull(const BoundingBox & bounds, const Vec3Array & vertices,
			const IndexArray & indices, const NormalArray & faceNormals,
			const IndexArray & planeIndices, const IndexArray & edgeIndices,
			const IndexArray & faceDirectionsIndices,
			const IndexArray & edgeDirectionsIndices);

	/**
	 * copy constructor
	 *
	 * @param other  hull to copy
	 */
	ConvexHull(const ConvexHull & other);

	/**
	 * destructor
	 */
	~ConvexHull();

	/**
	 * Collide box against hull
	 *
	 * @param box           box to collide against
	 * @param boxToThis     from box space to hull space
	 * @param intersection  intersection, written only if collision
	 *
	 * @return              true if collision, false otherwise
	 */
	bool collide(const BoundingBox & box, const Transform & boxToThis,
			Intersection & intersection) const;

	/**
	 * Collide two hulls
	 *
	 * @param other         hull to collide against
	 * @param otherToThis   from other hull space to this hull space
	 * @param intersection  intersection, written only if collision
	 *
	 * @return              true if collision, false otherwise
	 */
	bool collide(const ConvexHull & other, const Transform & otherToThis,
			Intersection & intersection) const;

	/**
	 * Collide hull against sphere
	 *
	 * @param point         point to collide against
	 * @param intersection  intersection, written only if collision
	 *
	 * @return              true if collision, false otherwise
	 */
	bool collide(const Sphere & sphere, const Transform & sphereToThis,
			Intersection & intersection) const;

	/**
	 * Test if point inside convex hull
	 *
	 * @param point  point to test, in convex hull space
	 *
	 * @return       true if point inside or on hull, false otherwise
	 */
	bool contains(const Vec3 & point) const;

	const BoundingBox & getBounds() const;

	/**
	 * Get unique edge directions
	 */
	const NormalArray & getEdgeDirections() const;

	const std::vector<Edge> & getEdges() const;

	/**
	 * Get unique face directions
	 */
	const NormalArray & getFaceDirections() const;

	/**
	 * get face normals of convex hull
	 *
	 * @return  face normals
	 */
	const NormalArray & getFaceNormals() const;

	/**
	 * Get planes of convex hull
	 */
	const std::vector<Plane> & getPlanes() const;

	const Sphere & getSphere() const;

	const TriangleList & getTriangleList() const;

	/**
	 * get vertices of convex hull
	 *
	 * @return  vertices of convex hull
	 */
	const Vec3Array & getVertices() const;

	/**
	 * Intersect against ray, write point and distance and return true if
	 * intersection occurs, otherwise return false leaving point and distance
	 * untouched
	 *
	 * @param ray       ray to intersect against
	 * @param point     written if intersection, point of intersection
	 * @param distance  written if intersection, distance from ray start to
	 *                  intersection
	 *
	 * @return          true if intersection occurred, false otherwise
	 */
	bool rayIntersection(const Ray & ray, Vec3 & point,
			double & distance) const;

	/**
	 * create version of convex hull transformed by matrix
	 *
	 * @param m  matrix to transform by
	 *
	 * @return   transformed convex hull
	 */
	ConvexHull transformed(const Mat4 & m) const;

	/**
	 * create version of convex hull transformed by transform
	 *
	 * @param transform  transform to transform by
	 *
	 * @return           transformed convex hull
	 */
	ConvexHull transformed(const Transform & transform) const;

	/**
	 * check convex hull loaded
	 *
	 * @return  true, if validation successful, false otherwise
	 */
	bool validate() const;

	/**
	 * get script object factory for ConvexHull
	 *
	 * @return  ConvexHull factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

