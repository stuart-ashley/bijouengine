#include "terrain.h"

#include "boundingBox.h"
#include "convexHull.h"
#include "indexArray.h"
#include "intersection.h"
#include "ray.h"
#include "sphere.h"
#include "transform.h"
#include "triangle.h"
#include "triangleList.h"
#include "vec3Array.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/string.h"

#include <algorithm>

namespace {
	/*
	 *
	 */
	std::vector<BaseParameter> params = { Parameter<String>("gridsize",
			nullptr), Parameter<Vec3Array>("vertices", nullptr), Parameter<
			IndexArray>("indices", nullptr) };

	/*
	 *
	 */
	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(params) {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			auto args = parameters.getArgs(nArgs, stack);

			float gridsize =
					std::static_pointer_cast<Real>(args["gridsize"])->getFloat();
			auto vertices = std::static_pointer_cast<Vec3Array>(
					args["vertices"]);
			auto indices = std::static_pointer_cast<IndexArray>(
					args["indices"]);

			stack.push(
					std::make_shared<Terrain>(gridsize, *vertices, *indices));
		}
	};

}

struct Terrain::impl {
	float gridSize;
	Vec3Array vertices;
	IndexArray indices;
	TriangleList triangles;
	BoundingBox bounds;
	std::vector<Ray> edges;
	bool valid;

	impl(float gridSize, const Vec3Array & vertices, const IndexArray & indices) :
					gridSize(gridSize),
					vertices(vertices),
					indices(indices),
					bounds(BoundingBox::empty()),
					valid(false) {
	}

	/**
	 * Clip terrain mesh against bounding box, triangles remain uncut
	 *
	 * @param box  bounding box to clip against
	 *
	 * @return     terrain clipped against box
	 */
	TriangleList clippedAndTransformed(const BoundingBox & box,
			const Transform & boxToTerrain) {

		auto boxBoundsInTerrainSpace = box.transformed(boxToTerrain);

		const auto & bmin = bounds.getMin();
		const auto & bmax = bounds.getMax();
		const auto & boxMin = boxBoundsInTerrainSpace.getMin();
		const auto & boxMax = boxBoundsInTerrainSpace.getMax();

		if (bmin.getX() > boxMax.getX() || boxMin.getX() > bmax.getX()
				|| bmin.getY() > boxMax.getY() || boxMin.getY() > bmax.getY()
				|| boxMin.getZ() > bmax.getZ()) {
			return std::vector<Triangle>();
		}

		int nCols = static_cast<int>((bmax.getX() - bmin.getX()) / gridSize
				+ .5f);
		int nRows = static_cast<int>((bmax.getY() - bmin.getY()) / gridSize
				+ .5f);

		int sCol = std::max(
				static_cast<int>((boxMin.getX() - bmin.getX()) / gridSize), 0);
		int sRow = std::max(
				static_cast<int>((boxMin.getY() - bmin.getY()) / gridSize), 0);

		int eCol = std::min(
				static_cast<int>((boxMax.getX() - bmin.getX()) / gridSize),
				nCols - 1);
		int eRow = std::min(
				static_cast<int>((boxMax.getY() - bmin.getY()) / gridSize),
				nRows - 1);

		// clipped & transformed
		auto terrainToHull = boxToTerrain.inverse();
		auto r = terrainToHull.getRotationMatrix();
		auto t = terrainToHull.getTranslation();
		TriangleList clippedAndTransformed;
		for (int i = sCol; i <= eCol; ++i) {
			for (int j = sRow; j <= eRow; ++j) {
				int idx = 2 * (j * nCols + i);
				auto ta = triangles.get(idx).rotatedTranslated(r, t);
				auto tb = triangles.get(idx + 1).rotatedTranslated(r, t);
				if (box.intersects(ta.getBounds())) {
					clippedAndTransformed.add(ta);
				}
				if (box.intersects(tb.getBounds())) {
					clippedAndTransformed.add(tb);
				}
			}
		}
		return clippedAndTransformed;
	}
};

/**
 * constructor
 *
 * @param gridSize
 * @param vertices
 * @param indices
 */
Terrain::Terrain(float gridSize, const Vec3Array & vertices,
		const IndexArray & indices) :
		pimpl(new impl(gridSize, vertices, indices)) {
}

/**
 * destructor
 */
Terrain::~Terrain() {
}

/**
 * Collide terrain with bounding box
 *
 * @param box           bounding box to intersect against
 * @param boxToTerrain  from box space to terrain space
 * @param intersection  intersection, written only if collision
 *
 * @return              true if collision, false otherwise
 */
bool Terrain::collide(const BoundingBox & box, const Transform & boxToTerrain,
		Intersection & intersection) {

	// clip terrain by hull and transform to hull space
	auto clippedTerrain = pimpl->clippedAndTransformed(box, boxToTerrain);
	if (clippedTerrain.size() == 0) {
		return false;
	}

	// test intersection
	if (clippedTerrain.intersection(box, intersection) == false) {
		return false;
	}

	// transform intersection from hull to terrain space
	intersection.transform(boxToTerrain);

	return true;
}

/**
 * Collide terrain with convex hull
 *
 * @param hull           other body to intersect against
 * @param hullToTerrain  from hull space to terrain space
 * @param intersection   intersection, written only if collision
 *
 * @return               true if collision, false otherwise
 */
bool Terrain::collide(const ConvexHull & hull, const Transform & hullToTerrain,
		Intersection & intersection) {

	// clip terrain by hull and transform to hull space
	auto clippedTerrain = pimpl->clippedAndTransformed(hull.getBounds(),
			hullToTerrain);
	if (clippedTerrain.size() == 0) {
		return false;
	}

	// test intersection
	if (clippedTerrain.intersection(hull, intersection) == false) {
		return false;
	}

	// transform intersection from hull to terrain space
	intersection.transform(hullToTerrain);

	return true;
}

/**
 * Collide terrain against sphere
 *
 * @param sphere        sphere to collide against
 * @param sphereToThis  from sphere space to terrain space
 * @param intersection  intersection, written only if collision
 *
 * @return              true if any part of sphere under terrain, false otherwise
 */
bool Terrain::collide(const Sphere & sphere, const Transform & sphereToThis,
		Intersection & intersection) {
	auto point = sphere.getPoint();
	sphereToThis.transformPoint(point);

	auto radius = sphere.getRadius();

	BoundingBox bb(
			Vec3(point.getX() - radius, point.getY() - radius,
					point.getZ() - radius),
			Vec3(point.getX() + radius, point.getY() + radius,
					point.getZ() + radius));

	// clip terrain by sphere
	auto clippedTerrain = pimpl->clippedAndTransformed(bb, Transform());
	if (clippedTerrain.size() == 0) {
		return false;
	}

	Vec3 intxnPoint;
	Normal intxnNormal(1.f, 0.f, 0.f); // initial value unimportant
	double intxnDepth = 0;

	// check planes
	Vec3 op;
	double dist;
	for (const auto & t : clippedTerrain) {
		auto dir = -t.getNormal();

		if (t.intersectRayExtended(point, dir, op, dist)) {
			dist = radius - dist;
			if (dist > intxnDepth) {
				intxnPoint = op;
				intxnNormal = dir;
				intxnDepth = dist;
			}
		}
	}

	if (false) {
		static Normal up(0.f, 0.f, 1.f);

		Vec3 v;
		// check edges
		for (const auto & r : pimpl->edges) {
			auto p = point - r.getStart();
			p.scaleAdd(p.dot(r.getDirection()), r.getDirection(), r.getStart());
			v = point - p;
			double d = v.length();
			if (d < intxnDepth) {
				intxnPoint = p;
				intxnNormal = v.normalized();
				intxnDepth = (v.dot(up) > 0) ? d : -d;
			}
		}

		// check points
		for (const auto & vertex : pimpl->vertices) {
			v = point - vertex;
			double d = v.length();
			if (d < intxnDepth) {
				intxnPoint = vertex;
				intxnNormal = v.normalized();
				intxnDepth = (v.dot(up) > 0) ? d : -d;
			}
		}
	}

	if (intxnDepth <= 0) {
		return false;
	}

	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	return true;
}

/**
 *
 * @return
 */
const BoundingBox & Terrain::getBounds() const {
	return pimpl->bounds;
}

/**
 *
 * @return
 */
const TriangleList & Terrain::getTriangleList() const {
	return pimpl->triangles;
}

/**
 * Validate vertices and indices, once valid create bounds and list of
 * triangles at which point vertices and indices become redundant
 *
 * @return  true if vertices and indices valid, false otherwise
 */
bool Terrain::validate() {
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
		const auto & a = pimpl->vertices.get(pimpl->indices.get(i));
		const auto & b = pimpl->vertices.get(pimpl->indices.get(i + 1));
		const auto & c = pimpl->vertices.get(pimpl->indices.get(i + 2));

		triangles.emplace_back(a, b, c);
	}
	pimpl->triangles = TriangleList(triangles);

	for (size_t i = 0, n = pimpl->indices.size(); i < n; i += 3) {
		auto a = pimpl->indices.get(i);
		auto b = pimpl->indices.get(i + 1);
		auto c = pimpl->indices.get(i + 2);

		if (a < b) {
			pimpl->edges.emplace_back(pimpl->vertices.get(a),
					pimpl->vertices.get(b));
		}
		if (b < c) {
			pimpl->edges.emplace_back(pimpl->vertices.get(b),
					pimpl->vertices.get(c));
		}
		if (c < a) {
			pimpl->edges.emplace_back(pimpl->vertices.get(c),
					pimpl->vertices.get(a));
		}
	}

	pimpl->bounds = pimpl->vertices.getBounds();
	return true;
}

/**
 * get script object factory for Terrain
 *
 * @return  Terrain factory
 */
STATIC const ScriptObjectPtr & Terrain::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
