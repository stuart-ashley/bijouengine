#include "parallelPlanes.h"

#include "boundingBox.h"
#include "convexHull.h"
#include "indexArray.h"
#include "intersection.h"
#include "plane.h"
#include "ray.h"
#include "rtree.h"
#include "transform.h"
#include "triangle.h"
#include "triangleList.h"
#include "vec3.h"
#include "vec3Array.h"

#include <algorithm>
#include <vector>

namespace {

	/*
	 *
	 */
	struct CollideCallback {
		TriangleList triangles;
		Mat3 r;
		Vec3 t;

		CollideCallback(const Transform & transform) :
				r(transform.getRotationMatrix()), t(transform.getTranslation()) {
		}

		void operator()(const Triangle & triangle) {
			triangles.add(triangle.rotatedTranslated(r, t));
		}
	};

	/*
	 *
	 */
	struct RayCallback {
		Ray ray;
		Vec3 point;
		double distance;
		bool hit;

		RayCallback(const Ray & ray) :
						ray(ray),
						distance(std::numeric_limits<double>::max()),
						hit(false) {
		}

		void operator()(const Triangle & triangle) {
			Vec3 p;
			double d;
			if (triangle.intersectRay(ray.getStart(), ray.getDirection(), p, d)
					&& d < distance) {
				point = p;
				distance = d;
				hit = true;
			}
		}
	};
}

struct ParallelPlanes::impl {
	Vec3Array vertices;
	IndexArray indices;
	TriangleList triangles;
	BoundingBox bounds;
	RTree<4, Triangle> rtree;
	bool valid;

	impl(const Vec3Array & vertices, const IndexArray & indices) :
					vertices(vertices),
					indices(indices),
					bounds(BoundingBox::empty()),
					valid(false) {
	}
};

/**
 *
 * @param vertices
 * @param indices
 */
ParallelPlanes::ParallelPlanes(const Vec3Array & vertices,
		const IndexArray & indices) :
		pimpl(new impl(vertices, indices)) {
}

/**
 * Collide ParallelPlanes with convex hull
 *
 * @param hull                  other body to intersect against
 * @param hullToParallelPlanes
 * @param intersection          intersection, written only if collision
 *
 * @return                      true if collision, false otherwise
 */
bool ParallelPlanes::collide(const ConvexHull & hull,
		const Transform & hullToParallelPlanes,
		Intersection & intersection) const {
	/*
	 * Intersect planes space bounds of hull against pre-calculated r-tree
	 * of triangles. Transform each intersected triangle into hull space,
	 * then add to list. Intersect hull against pruned list of hull space
	 * triangles
	 */

	auto planesToHull = hullToParallelPlanes.inverse();
	CollideCallback cb(planesToHull);

	// hull bounds in plane space
	auto hullBounds = hull.getBounds().transformed(hullToParallelPlanes);

	pimpl->rtree.intersect(hullBounds, cb);

	if (cb.triangles.size() == 0) {
		return false;
	}

	// intersection
	if (cb.triangles.intersection(hull, intersection) == false) {
		return false;
	}

	intersection.transform(hullToParallelPlanes);
	return true;
}

/**
 *
 * @return
 */
const BoundingBox & ParallelPlanes::getBounds() const {
	return pimpl->bounds;
}

/**
 *
 * @return
 */
const TriangleList & ParallelPlanes::getTriangleList() const {
	return pimpl->triangles;
}

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
bool ParallelPlanes::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	/*
	 * Calculate ray against triangle intersection for each r-tree intersect.
	 * Keep nearest intersection point and distance
	 */

	double tmin = 0;
	double tmax = std::numeric_limits<double>::max();

	// intersection of ray and planes
	RayCallback cb(ray);
	pimpl->rtree.rayIntersect(ray, tmin, tmax, cb);

	if (cb.hit == false) {
		return false;
	}

	point = cb.point;
	distance = cb.distance;
	return true;
}

/**
 * Validate vertices and indices, once valid create bounds and list of
 * triangles at which point vertices and indices become redundant
 *
 * @return  true if vertices and indices valid, false otherwise
 */
bool ParallelPlanes::validate() {
	if (pimpl->valid) {
		return true;
	}
	if (pimpl->indices.validate() == false) {
		return false;
	}
	if (pimpl->vertices.validate() == false) {
		return false;
	}

	std::vector<Triangle> triangles;
	for (size_t i = 0, n = pimpl->indices.size(); i < n; i += 3) {
		Vec3 a = pimpl->vertices.get(pimpl->indices.get(i));
		Vec3 b = pimpl->vertices.get(pimpl->indices.get(i + 1));
		Vec3 c = pimpl->vertices.get(pimpl->indices.get(i + 2));

		triangles.emplace_back(a, b, c);
	}
	for (const auto & triangle : triangles) {
		pimpl->rtree.insert(triangle.getBounds(), triangle);
	}
	pimpl->triangles = TriangleList(triangles);

	pimpl->bounds = pimpl->vertices.getBounds();
	pimpl->valid = true;
	return true;
}
