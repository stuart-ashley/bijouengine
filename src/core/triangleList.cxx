#include "triangleList.h"

#include "convexHull.h"
#include "indexArray.h"
#include "intersection.h"
#include "plane.h"
#include "ray.h"
#include "vec3Array.h"

#include <limits>

namespace {
	const double e = 0.001;

	/**
	* split triangle based on depths of vertices, adding below part to
	* vector of triangles
	*
	* @param triangle  triangle to split
	* @param d0        depth of 1st vertex
	* @param d1        depth of 2nd vertex
	* @param d2        depth of 3rd vertex
	* @param below     target triangle list
	*/
	void splitTriangle(const Triangle & triangle, double d0, double d1,
			double d2, std::vector<Triangle> & below) {

		// get triangle vertices and normal
		const auto & v0 = triangle.getVertex0();
		const auto & v1 = triangle.getVertex1();
		const auto & v2 = triangle.getVertex2();
		const auto & n = triangle.getNormal();

		if (d0 > -e && d1 > -e && d2 > -e) {
			// above
			return;
		}
		if (d0 < e && d1 < e && d2 < e) {
			// below
			below.emplace_back(triangle);
			return;
		}

		if (d0 > -e && d0 < e) {
			// cut edge bc
			Vec3 p;
			p.interpolate(v1, v2, d1 / (d1 - d2));
			if (d1 < 0) {
				below.emplace_back(v0, v1, p, n);
			}
			else {
				below.emplace_back(p, v2, v0, n);
			}
			return;
		}
		if (d1 > -e && d1 < e) {
			// cut edge ca
			Vec3 p;
			p.interpolate(v2, v0, d2 / (d2 - d0));
			if (d0 < 0) {
				below.emplace_back(v0, v1, p, n);
			}
			else {
				below.emplace_back(v1, v2, p, n);
			}
			return;
		}
		if (d2 > -e && d2 < e) {
			// cut edge ab
			Vec3 p;
			p.interpolate(v0, v1, d0 / (d0 - d1));
			if (d0 < 0) {
				below.emplace_back(v0, p, v2, n);
			}
			else {
				below.emplace_back(p, v1, v2, n);
			}
			return;
		}

		bool aBelow = d0 < 0 && d1 > 0 && d2 > 0;
		bool aAbove = d0 > 0 && d1 < 0 && d2 < 0;
		if (aBelow || aAbove) {
			// cut edge ab and ca
			Vec3 p, q;
			p.interpolate(v0, v1, d0 / (d0 - d1));
			q.interpolate(v2, v0, d2 / (d2 - d0));
			if (aBelow) {
				below.emplace_back(v0, p, q, n);
			}
			else {
				below.emplace_back(p, v1, v2, n);
				below.emplace_back(p, v2, q, n);
			}
			return;
		}
		bool bBelow = d0 > 0 && d1 < 0 && d2 > 0;
		bool bAbove = d0 < 0 && d1 > 0 && d2 < 0;
		if (bBelow || bAbove) {
			// cut edge ab and bc
			Vec3 p, q;
			p.interpolate(v0, v1, d0 / (d0 - d1));
			q.interpolate(v1, v2, d1 / (d1 - d2));
			if (bBelow) {
				below.emplace_back(p, v1, q, n);
			}
			else {
				below.emplace_back(v0, p, v2, n);
				below.emplace_back(p, q, v2, n);
			}
			return;
		}
		bool cBelow = d0 > 0 && d1 > 0 && d2 < 0;
		bool cAbove = d0 < 0 && d1 < 0 && d2 > 0;
		if (cBelow || cAbove) {
			// cut edge ca and bc
			Vec3 p, q;
			p.interpolate(v2, v0, d2 / (d2 - d0));
			q.interpolate(v1, v2, d1 / (d1 - d2));
			if (cBelow) {
				below.emplace_back(p, q, v2, n);
			}
			else {
				below.emplace_back(v0, q, p, n);
				below.emplace_back(v0, v1, q, n);
			}
		}
	}

	/**
	* Calculate intersection of mesh against bounding box
	*
	* @param box  bounding box to clip against
	*
	* @return     list of triangles wholly inside bounding box
	*/
	std::vector<Triangle> intersectionTriangles(
			const std::vector<Triangle> & triangles, const BoundingBox & box) {

		std::vector<Triangle> tmp[2];

		auto min = box.getMin();
		auto max = box.getMax();

		auto maxx = max.getX();
		for (const auto & triangle : triangles) {
			auto d0 = triangle.getVertex0().getX() - maxx;
			auto d1 = triangle.getVertex1().getX() - maxx;
			auto d2 = triangle.getVertex2().getX() - maxx;

			splitTriangle(triangle, d0, d1, d2, tmp[0]);
		}
		
		auto minx = min.getX();
		for (const auto & triangle : tmp[0]) {
			auto d0 = minx - triangle.getVertex0().getX();
			auto d1 = minx - triangle.getVertex1().getX();
			auto d2 = minx - triangle.getVertex2().getX();

			splitTriangle(triangle, d0, d1, d2, tmp[1]);
		}

		tmp[0].clear();
		auto maxy = max.getY();
		for (const auto & triangle : tmp[1]) {
			auto d0 = triangle.getVertex0().getY() - maxy;
			auto d1 = triangle.getVertex1().getY() - maxy;
			auto d2 = triangle.getVertex2().getY() - maxy;

			splitTriangle(triangle, d0, d1, d2, tmp[0]);
		}

		tmp[1].clear();
		auto miny = min.getY();
		for (const auto & triangle : tmp[0]) {
			auto d0 = miny - triangle.getVertex0().getY();
			auto d1 = miny - triangle.getVertex1().getY();
			auto d2 = miny - triangle.getVertex2().getY();

			splitTriangle(triangle, d0, d1, d2, tmp[1]);
		}

		tmp[0].clear();
		auto maxz = max.getZ();
		for (const auto & triangle : tmp[1]) {
			auto d0 = triangle.getVertex0().getZ() - maxz;
			auto d1 = triangle.getVertex1().getZ() - maxz;
			auto d2 = triangle.getVertex2().getZ() - maxz;

			splitTriangle(triangle, d0, d1, d2, tmp[0]);
		}

		tmp[1].clear();
		auto minz = min.getZ();
		for (const auto & triangle : tmp[0]) {
			auto d0 = minz - triangle.getVertex0().getZ();
			auto d1 = minz - triangle.getVertex1().getZ();
			auto d2 = minz - triangle.getVertex2().getZ();

			splitTriangle(triangle, d0, d1, d2, tmp[1]);
		}


		return tmp[1];
	}

	/**
	* Calculate intersection of mesh against convex hull
	*
	* @param hull  convex hull to clip against
	*
	* @return      list of triangles wholly inside convex hull
	*/
	std::vector<Triangle> intersectionTriangles(
			const std::vector<Triangle> & triangles, const ConvexHull & hull) {

		std::vector<Triangle> tmp[2];
		int active = 0;

		const auto * working = &triangles;

		for (const auto & plane : hull.getPlanes()) {
			auto & cutTris = tmp[active];
			active = 1 - active;
			cutTris.clear();
			for (const auto & triangle : *working) {
				// distances from triangle
				double d0 = plane.distanceTo(triangle.getVertex0());
				double d1 = plane.distanceTo(triangle.getVertex1());
				double d2 = plane.distanceTo(triangle.getVertex2());

				splitTriangle(triangle, d0, d1, d2, cutTris);
			}
			if (cutTris.size() == 0) {
				return cutTris;
			}
			working = &cutTris;
		}
		return *working;
	}
}

/**
 * empty constructor
 */
TriangleList::TriangleList() {
}

/**
 * constructor from vector
 *
 * @param triangles
 */
TriangleList::TriangleList(const std::vector<Triangle> & triangles) :
		triangles(triangles) {
}

/**
 * construct from vertices & indices
 *
 * @param indices
 * @param vertices
 */
TriangleList::TriangleList(const IndexArray & indices,
		const Vec3Array & vertices) {
	size_t n = indices.size();
	triangles.reserve(n / 3);

	for (size_t i = 0; i < n; i += 3) {
		triangles.emplace_back(vertices.get(indices.get(i)),
				vertices.get(indices.get(i + 1)),
				vertices.get(indices.get(i + 2)));
	}
}

/**
* Calculate intersection of triangle list against bounding box
*
* @param box           bounding box to intersect against
* @param intersection  intersection, written only if intersection
*
* @return              true if intersection, false otherwise
*/
bool TriangleList::intersection(const BoundingBox & box,
		Intersection & intersection) const {
	auto tris = intersectionTriangles(triangles, box);
	if (tris.size() == 0) {
		return false;
	}
	Vec3 p, tmp;
	double area = 0;
	for (const auto & t : tris) {
		auto ta = t.getArea();
		area += ta;
		p.scaleAdd(ta, t.getCentre(), p);
		tmp.scaleAdd(ta, t.getNormal(), tmp);
	}
	p = p / area;
	auto n = tmp.normalized();

	double maxDist = -std::numeric_limits<double>::max();

	auto min = box.getMin();
	auto max = box.getMax();
	Vec3 boxVertices[] = {
			Vec3(min.getX(), min.getY(), min.getZ()),
			Vec3(min.getX(), min.getY(), max.getZ()),
			Vec3(min.getX(), max.getY(), min.getZ()),
			Vec3(min.getX(), max.getY(), max.getZ()),
			Vec3(max.getX(), min.getY(), min.getZ()),
			Vec3(max.getX(), min.getY(), max.getZ()),
			Vec3(max.getX(), max.getY(), min.getZ()),
			Vec3(max.getX(), max.getY(), max.getZ()) };

	for (const auto & t : triangles) {
		for (const auto & v : boxVertices) {
			Vec3 q;
			double d = 0;

			if (t.intersectRay(v, n, q, d) && d > maxDist) {
				maxDist = d;
			}
		}
	}

	if (maxDist == -std::numeric_limits<double>::max()) {
		return false;
	}

	intersection.set(p, -n, maxDist);
	return true;
}

/**
* Calculate intersection of triangle list against convex hull
*
* @param hull          convex hull to intersect against
* @param intersection  intersection, written only if intersection
*
* @return              true if intersection, false otherwise
*/
bool TriangleList::intersection(const ConvexHull & hull,
		Intersection & intersection) const {
	auto tris = intersectionTriangles(triangles, hull);
	if (tris.size() == 0) {
		return false;
	}
	Vec3 p, tmp;
	double area = 0;
	for (const auto & t : tris) {
		auto ta = t.getArea();
		area += ta;
		p.scaleAdd(ta, t.getCentre(), p);
		tmp.scaleAdd(ta, t.getNormal(), tmp);
	}
	p = p / area;
	auto n = tmp.normalized();

	double maxDist = -std::numeric_limits<double>::max();

	for (const auto & t : triangles) {
		for (const auto & v : hull.getVertices()) {
			Vec3 q;
			double d = 0;

			if (t.intersectRay(v, n, q, d) && d > maxDist) {
				maxDist = d;
			}
		}
	}

	if (maxDist == -std::numeric_limits<double>::max()) {
		return false;
	}

	intersection.set(p, -n, maxDist);
	return true;
}

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
bool TriangleList::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	// maximum negative distance ( closest to zero and negative )
	double maxNegativeDistance = -std::numeric_limits<double>::max();
	Vec3 maxNegativePoint;

	// minimum positive distance ( closest to zero and positive )
	double minPositiveDistance = std::numeric_limits<double>::max();
	Vec3 minPositivePoint;

	int negativeDistanceCount = 0;

	for (const auto & t : triangles) {
		Vec3 p;
		double d;

		if (t.intersectsExtendedLineSegment(ray.getStart(), ray.getEnd(), p, d)
				== false) {
			// no intersection
			continue;
		}

		if (d < 0.0) {
			negativeDistanceCount++;
			if (maxNegativeDistance < d) {
				maxNegativeDistance = d;
				maxNegativePoint = p;
			}
		} else {
			if (minPositiveDistance > d) {
				minPositiveDistance = d;
				minPositivePoint = p;
			}
		}
	}

	if (negativeDistanceCount % 2 == 1) {
		// unchanged, no intersection
		if (maxNegativeDistance == -std::numeric_limits<double>::max()) {
			return false;
		}
		// ray start inside
		distance = maxNegativeDistance;
		point = maxNegativePoint;
		return true;
	} else {
		// unchanged, no intersection
		if (minPositiveDistance == std::numeric_limits<double>::max()) {
			return false;
		}
		// ray start outside
		distance = minPositiveDistance;
		point = minPositivePoint;
	}

	return true;
}

