#include "convexHull.h"

#include "boundingBox.h"
#include "indexArray.h"
#include "intersection.h"
#include "mat4.h"
#include "normalArray.h"
#include "ray.h"
#include "sphere.h"
#include "transform.h"
#include "triangle.h"
#include "triangleList.h"
#include "vec3.h"
#include "vec3Array.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

#include <cassert>
#include <limits>
#include <mutex>

namespace {
	std::vector<BaseParameter> params = {
			Parameter<BoundingBox>("bounds", nullptr),
			Parameter<Vec3Array>("vertices", nullptr),
			Parameter<IndexArray>("indices", nullptr),
			Parameter<NormalArray>("faceNormals", nullptr),
			Parameter<IndexArray>("planes", nullptr),
			Parameter<IndexArray>("edges", nullptr),
			Parameter<IndexArray>("faceDirections", nullptr),
			Parameter<IndexArray>("edgeDirections", nullptr) };
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
			auto bounds = std::static_pointer_cast<BoundingBox>(args["bounds"]);
			auto vertices = std::static_pointer_cast<Vec3Array>(
					args["vertices"]);
			auto indices = std::static_pointer_cast<IndexArray>(
					args["indices"]);
			auto faceNormals = std::static_pointer_cast<NormalArray>(
					args["faceNormals"]);
			auto planes = std::static_pointer_cast<IndexArray>(args["planes"]);
			auto edges = std::static_pointer_cast<IndexArray>(args["edges"]);
			auto faceDirections = std::static_pointer_cast<IndexArray>(
					args["faceDirections"]);
			auto edgeDirections = std::static_pointer_cast<IndexArray>(
					args["edgeDirections"]);
			stack.emplace(
					std::make_shared<ConvexHull>(*bounds, *vertices, *indices,
							*faceNormals, *planes, *edges, *faceDirections,
							*edgeDirections));
		}
	};

	/*
	 * Bounding box static data
	 */
	namespace box {
		/** bounding box triangle indices */
		IndexArray indices = { 0, 1, 3, 0, 3, 2, 0, 4, 5, 0, 5, 1, 0, 2, 6, 0,
				6, 4, 1, 5, 7, 1, 7, 3, 4, 6, 7, 4, 7, 5, 2, 3, 7, 2, 7, 6 };

		/** bounding box face normals */
		NormalArray faceNormals = {
				Normal(-1.f, 0.f, 0.f),
				Normal(0.f, -1.f, 0.f),
				Normal(0.f, 0.f, -1.f),
				Normal(0.f, 0.f, 1.f),
				Normal(1.f, 0.f, 0.f),
				Normal(0.f, 1.f, 0.f) };

		/**
		 * bounding box edge indices, for each edge, 2 vertex indices and 2 face
		 * normal indices
		 */
		IndexArray edgeIndices = { 0, 1, 0, 1, 0, 2, 0, 2, 1, 3, 0, 3, 2, 3, 0,
				5, 0, 4, 1, 2, 1, 5, 1, 3, 4, 5, 1, 4, 4, 6, 2, 4, 2, 6, 2, 5,
				5, 7, 3, 4, 3, 7, 3, 5, 6, 7, 4, 5 };

		/**
		 * bounding box plane point indices, index into vertices retrieving a point
		 * on the plane of each face normal
		 */
		IndexArray planeIndices = { 0, 0, 0, 1, 4, 2 };

		/**
		 * bounding box face direction indices, index into face normals retrieving
		 * unique face direction
		 */
		IndexArray faceDirectionsIndices = { 0, 1, 2 };

		/** bounding box edge direction indices, index into edges */
		IndexArray edgeDirectionsIndices = { 0, 1, 4 };

		/** calculate bounding box corners */
		Vec3Array corners(const BoundingBox & box) {
			const auto & min = box.getMin();
			const auto & max = box.getMax();
			return Vec3Array{
					Vec3(min.getX(), min.getY(), min.getZ()),
					Vec3(min.getX(), min.getY(), max.getZ()),
					Vec3(min.getX(), max.getY(), min.getZ()),
					Vec3(min.getX(), max.getY(), max.getZ()),
					Vec3(max.getX(), min.getY(), min.getZ()),
					Vec3(max.getX(), min.getY(), max.getZ()),
					Vec3(max.getX(), max.getY(), min.getZ()),
					Vec3(max.getX(), max.getY(), max.getZ()) };
		}

		Normal directions[] = {
				Normal(1.f, 0.f, 0.f),
				Normal(0.f, 1.f, 0.f),
				Normal(0.f, 0.f, 1.f) };
	}
}

struct ConvexHull::impl {
	std::mutex m_edgeDirectionsLock;
	std::mutex m_edgesLock;
	std::mutex m_faceDirectionsLock;
	std::mutex m_planesLock;
	std::mutex m_sphereLock;
	std::mutex m_triangleListLock;
	std::mutex m_validateLock;

	BoundingBox bounds;

	Vec3Array vertices;
	IndexArray indices;
	NormalArray faceNormals;
	IndexArray planeIndices;
	IndexArray edgeIndices;
	IndexArray faceDirectionIndices;
	IndexArray edgeDirectionIndices;

	/** planes forming convex hull normals pointing outward */
	std::unique_ptr<std::vector<Plane>> planes;
	std::unique_ptr<std::vector<Edge>> edges;
	std::unique_ptr<NormalArray> faceDirections;
	std::unique_ptr<NormalArray> edgeDirections;
	std::unique_ptr<TriangleList> triangleList;
	std::unique_ptr<Sphere> sphere;

	impl(const BoundingBox & box) :
					bounds(box),
					vertices(box::corners(box)),
					indices(box::indices),
					faceNormals(box::faceNormals),
					planeIndices(box::planeIndices),
					edgeIndices(box::edgeIndices),
					faceDirectionIndices(box::faceDirectionsIndices),
					edgeDirectionIndices(box::edgeDirectionsIndices) {
	}

	impl(const BoundingBox & bounds, const Vec3Array & vertices,
			const IndexArray & indices, const NormalArray & faceNormals,
			const IndexArray & planeIndices, const IndexArray & edgeIndices,
			const IndexArray & faceDirectionsIndices,
			const IndexArray & edgeDirectionsIndices) :
					bounds(bounds),
					vertices(vertices),
					indices(indices),
					faceNormals(faceNormals),
					planeIndices(planeIndices),
					edgeIndices(edgeIndices),
					faceDirectionIndices(faceDirectionsIndices),
					edgeDirectionIndices(edgeDirectionsIndices) {
	}

	impl(const impl & other) :
					bounds(other.bounds),
					vertices(other.vertices),
					indices(other.indices),
					faceNormals(other.faceNormals),
					planeIndices(other.planeIndices),
					edgeIndices(other.edgeIndices),
					faceDirectionIndices(other.faceDirectionIndices),
					edgeDirectionIndices(other.edgeDirectionIndices) {
	}
};

/**
 * Construct from bounding box
 */
EXPLICIT ConvexHull::ConvexHull(const BoundingBox & box) :
		pimpl(new impl(box)) {
}

/**
 * Constructor
 */
ConvexHull::ConvexHull(const BoundingBox & bounds, const Vec3Array & vertices,
		const IndexArray & indices, const NormalArray & faceNormals,
		const IndexArray & planeIndices, const IndexArray & edgeIndices,
		const IndexArray & faceDirectionsIndices,
		const IndexArray & edgeDirectionsIndices) :
				pimpl(new impl(bounds, vertices, indices, faceNormals,
						planeIndices, edgeIndices, faceDirectionsIndices,
						edgeDirectionsIndices)) {
}

/**
 * copy constructor
 *
 * @param other  hull to copy
 */
ConvexHull::ConvexHull(const ConvexHull & other) :
		pimpl(new impl(*other.pimpl)) {
}

/**
 * destructor
 */
ConvexHull::~ConvexHull() {
}

/**
 * Collide box against hull
 *
 * @param box           box to collide against
 * @param boxToThis     from box space to hull space
 * @param intersection  intersection, written only if collision
 *
 * @return              true if collision, false otherwise
 */
bool ConvexHull::collide(const BoundingBox & box, const Transform & boxToThis,
		Intersection & intersection) const {

	/* test bounds */
	auto bb = box.transformed(boxToThis);
	if (pimpl->bounds.intersects(bb) == false) {
		return false;
	}

	auto boxVertices = box::corners(box);

	auto boxVerticesInThisSpace = boxVertices.transformed(boxToThis);

	Vec3 minPoint, maxPoint;
	double minDist, maxDist;
	Vec3 minPoint2, maxPoint2;
	double minDist2, maxDist2;

	double intxnDepth = std::numeric_limits<double>::max();
	Normal intxnNormal(0.f, 0.f, 1.f);
	Vec3 intxnPoint;
	bool intxnTransform = false;

	// 'this' face normals
	for (const auto & axis : getFaceDirections()) {
		// extremes for axis
		pimpl->vertices.getDirectionMinMax(axis, minDist, maxDist);
		boxVerticesInThisSpace.getDirectionMinMax(axis, minPoint2, maxPoint2,
				minDist2, maxDist2);

		// check for gap
		if (minDist > maxDist2 || minDist2 > maxDist) {
			return false;
		}

		// update
		if (minDist < minDist2) {
			if (maxDist - minDist2 < intxnDepth) {
				intxnDepth = maxDist - minDist2;
				intxnNormal = -axis;
				intxnPoint = minPoint2;
			}
		} else {
			if (maxDist2 - minDist < intxnDepth) {
				intxnDepth = maxDist2 - minDist;
				intxnNormal = axis;
				intxnPoint = maxPoint2;
			}
		}
	}

	// 'this' space in 'that' space
	auto thisToBox = boxToThis.inverse();
	auto thisVerticesInBoxSpace = pimpl->vertices.transformed(thisToBox);

	double boxMinDist[3] = { box.getMin().getX(), box.getMin().getY(),
			box.getMin().getZ() };
	double boxMaxDist[3] = { box.getMax().getX(), box.getMax().getY(),
			box.getMax().getZ() };

	// 'that' face normals
	for (int i = 0; i < 3; ++i) {
		// extremes for axis
		thisVerticesInBoxSpace.getDirectionMinMax(box::directions[i], minPoint2,
				maxPoint2, minDist2, maxDist2);

		// check for gap
		if (boxMinDist[i] > maxDist2 || minDist2 > boxMaxDist[i]) {
			return false;
		}

		// update
		if (boxMinDist[i] < minDist2) {
			if (boxMaxDist[i] - minDist2 < intxnDepth) {
				intxnDepth = boxMaxDist[i] - minDist2;
				intxnNormal = box::directions[i];
				intxnPoint = minPoint2;
				intxnTransform = true;
			}
		} else {
			if (maxDist2 - boxMinDist[i] < intxnDepth) {
				intxnDepth = maxDist2 - boxMinDist[i];
				intxnNormal = -box::directions[i];
				intxnPoint = maxPoint2;
				intxnTransform = true;
			}
		}
	}

	// edge cross
	auto rotateThisToBox = thisToBox.getRotationMatrix();
	for (const auto & dir : getEdgeDirections()) {
		// 'dir' into 'that' space
		auto a = rotateThisToBox * dir;

		Normal axes[] = {
				Normal(0, a.getZ(), -a.getY()),
				Normal(-a.getZ(), 0, a.getX()),
				Normal(a.getY(),-a.getX(), 0) };

		for (int i = 0; i < 3; ++i) {
			if (axes[i].invalid()) {
				continue;
			}

			// extremes for axis
			box.getDirectionMinMax(axes[i], minPoint, maxPoint);
			minDist = minPoint.dot(axes[i]);
			maxDist = maxPoint.dot(axes[i]);
			thisVerticesInBoxSpace.getDirectionMinMax(axes[i], minPoint2,
					maxPoint2, minDist2, maxDist2);

			// check for gap
			if (minDist > maxDist2 || minDist2 > maxDist) {
				return false;
			}

			// update
			if (minDist < minDist2) {
				if (maxDist - minDist2 < intxnDepth) {
					intxnDepth = maxDist - minDist2;
					intxnNormal = axes[i];
					intxnPoint = minPoint2;
					intxnTransform = true;
				}
			} else {
				if (maxDist2 - minDist < intxnDepth) {
					intxnDepth = maxDist2 - minDist;
					intxnNormal = -axes[i];
					intxnPoint = maxPoint2;
					intxnTransform = true;
				}
			}
		}
	}

	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	if (intxnTransform) {
		intersection.transform(boxToThis);
	}
	return true;
}

/**
 * Collide two hulls
 *
 * @param other         hull to collide against
 * @param otherToThis   from other hull space to this hull space
 * @param intersection  intersection, written only if collision
 *
 * @return              true if collision, false otherwise
 */
bool ConvexHull::collide(const ConvexHull & other,
		const Transform & otherToThis, Intersection & intersection) const {
	/* test bounds */
	auto bb = other.pimpl->bounds.transformed(otherToThis);
	if (pimpl->bounds.intersects(bb) == false) {
		return false;
	}

	auto thatVerticesInThisSpace = other.pimpl->vertices.transformed(
			otherToThis);

	double minDist, maxDist;
	Vec3 minPoint2, maxPoint2;
	double minDist2, maxDist2;

	double intxnDepth = std::numeric_limits<double>::max();
	Normal intxnNormal(0.f, 0.f, 1.f);
	Vec3 intxnPoint;
	bool intxnTransform = false;

	// 'this' face normals
	for (const auto & axis : getFaceDirections()) {
		// extremes for axis
		pimpl->vertices.getDirectionMinMax(axis, minDist, maxDist);
		thatVerticesInThisSpace.getDirectionMinMax(axis, minPoint2, maxPoint2,
				minDist2, maxDist2);

		// check for gap
		if (minDist > maxDist2 || minDist2 > maxDist) {
			return false;
		}

		// update
		if (minDist < minDist2) {
			if (maxDist - minDist2 < intxnDepth) {
				intxnDepth = maxDist - minDist2;
				intxnNormal = -axis;
				intxnPoint = minPoint2;
			}
		} else {
			if (maxDist2 - minDist < intxnDepth) {
				intxnDepth = maxDist2 - minDist;
				intxnNormal = axis;
				intxnPoint = maxPoint2;
			}
		}
	}

	// 'this' space in 'that' space
	auto thisToThat = otherToThis.inverse();
	auto thisVerticesInThatSpace = pimpl->vertices.transformed(thisToThat);

	// 'that' face normals
	for (const auto & axis : other.getFaceDirections()) {
		// extremes for axis
		other.pimpl->vertices.getDirectionMinMax(axis, minDist, maxDist);
		thisVerticesInThatSpace.getDirectionMinMax(axis, minPoint2, maxPoint2,
				minDist2, maxDist2);

		// check for gap
		if (minDist > maxDist2 || minDist2 > maxDist) {
			return false;
		}

		// update
		if (minDist < minDist2) {
			if (maxDist - minDist2 < intxnDepth) {
				intxnDepth = maxDist - minDist2;
				intxnNormal = axis;
				intxnPoint = minPoint2;
				intxnTransform = true;
			}
		} else {
			if (maxDist2 - minDist < intxnDepth) {
				intxnDepth = maxDist2 - minDist;
				intxnNormal = -axis;
				intxnPoint = maxPoint2;
				intxnTransform = true;
			}
		}
	}

	// edge cross
	auto rotateThisToThat = thisToThat.getRotationMatrix();
	const auto & otherEdgeDirections = other.getEdgeDirections();
	for (const auto & dir : getEdgeDirections()) {
		// 'dir' into 'that' space
		auto a = rotateThisToThat * dir;

		for (const auto & b : otherEdgeDirections) {
			auto axis = a.cross(b);
			if (axis.invalid()) {
				continue;
			}

			// extremes for axis
			other.pimpl->vertices.getDirectionMinMax(axis, minDist, maxDist);
			thisVerticesInThatSpace.getDirectionMinMax(axis, minPoint2,
					maxPoint2, minDist2, maxDist2);

			// check for gap
			if (minDist > maxDist2 || minDist2 > maxDist) {
				return false;
			}

			// update
			if (minDist < minDist2) {
				if (maxDist - minDist2 < intxnDepth) {
					intxnDepth = maxDist - minDist2;
					intxnNormal = axis;
					intxnPoint = minPoint2;
					intxnTransform = true;
				}
			} else {
				if (maxDist2 - minDist < intxnDepth) {
					intxnDepth = maxDist2 - minDist;
					intxnNormal = -axis;
					intxnPoint = maxPoint2;
					intxnTransform = true;
				}
			}
		}
	}

	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	if (intxnTransform) {
		intersection.transform(otherToThis);
	}
	return true;
}

/**
 * Collide hull against sphere
 *
 * @param point         point to collide against
 * @param intersection  intersection, written only if collision
 *
 * @return              true if collision, false otherwise
 */
bool ConvexHull::collide(const Sphere & sphere, const Transform & sphereToThis,
		Intersection & intersection) const {
	double intxnDepth = std::numeric_limits<double>::max();
	Normal intxnNormal(0.f, 0.f, 1.f);
	Vec3 intxnPoint;

	double intxnDepth2 = std::numeric_limits<double>::max();
	Normal intxnNormal2(0.f, 0.f, 1.f);
	Vec3 intxnPoint2;

	auto point = sphere.getPoint();
	auto radius = sphere.getRadius();
	sphereToThis.transformPoint(point);

	const auto & s = getSphere();
	Vec3 v = s.getPoint() - point;
	if (v.length() > s.getRadius() + radius) {
		return false;
	}

	// test faces
	bool inside = true;
	for (const auto & t : getTriangleList()) {
		v = point - t.getVertex0();
		Normal n = t.getNormal();
		double d = v.dot(n);
		if (d > radius) {
			return false;
		}
		if (d < 0) {
			// potentially closer intersection
			if (radius - d < intxnDepth2) {
				// project onto plane
				v.scaleAdd(-d, n, point);
				// in triangle
				if (t.contains(v)) {
					intxnDepth2 = radius - d;
					intxnNormal2 = -n;
					intxnPoint2.scaleAdd(-(radius + d) / 2, n, point);
				}
			}
		} else {
			inside = false;
			// potentially closer intersection
			if (radius - d < intersection.getDepth()) {
				// project onto plane
				v.scaleAdd(-d, n, point);
				// in triangle
				if (t.contains(v)) {
					intxnDepth = radius - d;
					intxnNormal = -n;
					intxnPoint.scaleAdd(-(radius + d) / 2, n, point);
				}
			}
		}
	}
	if (inside) {
		intersection.set(intxnPoint2, intxnNormal2, intxnDepth2);
		return true;
	}

	// test edges
	for (const auto & e : getEdges()) {
		Vec3 a = pimpl->vertices.get(e.getVertexIndex0());
		Vec3 b = pimpl->vertices.get(e.getVertexIndex1());
		// edge direction
		v = b - a;
		double l = v.length();
		Normal n = v.normalized();
		// distance along edge
		v = point - a;
		double d = v.dot(n);
		// check on edge
		if (d > 0 && d < l) {
			// project onto line
			v.scaleAdd(d, n, a);
			// distance to point
			Vec3 tmp = v - point;
			d = tmp.length();
			// closer intersection
			if (d < radius && radius - d < intersection.getDepth()) {
				intxnDepth = radius - d;
				intxnNormal = tmp.normalized();
				intxnPoint.scaleAdd((radius + d) / 2, intxnNormal, point);
			}
		}
	}

	// test vertices
	for (const auto & vertex : pimpl->vertices) {
		v = vertex - point;
		double d = v.length();
		Normal n = v.normalized();
		// closer intersection
		if (d < radius && d < intersection.getDepth()) {
			intxnDepth = radius - d;
			intxnNormal = n;
			intxnPoint.scaleAdd((radius + d) / 2, n, point);
		}
	}

	if (intxnDepth == std::numeric_limits<double>::max()) {
		return false;
	}
	intersection.set(intxnPoint, intxnNormal, intxnDepth);
	return true;
}

/**
 * Test if point inside convex hull
 *
 * @param point  point to test, in convex hull space
 *
 * @return       true if point inside or on hull, false otherwise
 */
bool ConvexHull::contains(const Vec3 & point) const {
	for (const auto & p : getPlanes()) {
		double d = p.distanceTo(point);
		if (d > 0.f) {
			return false;
		}
	}
	return true;
}

/**
 * get bounds of hull
 *
 * @return  hull bounding box
 */
const BoundingBox & ConvexHull::getBounds() const {
	return pimpl->bounds;
}

/**
 * Get unique edge directions
 */
const NormalArray & ConvexHull::getEdgeDirections() const {
	const double epsilon = .00001;

	std::lock_guard<std::mutex> locker(pimpl->m_edgeDirectionsLock);

	if (pimpl->edgeDirections == nullptr) {
		auto nEdgeDirections = pimpl->edgeDirectionIndices.size();
		std::vector<Normal> edgeDirections;
		edgeDirections.reserve(nEdgeDirections);

		for (const auto & e : getEdges()) {
			auto v = pimpl->vertices.get(e.getVertexIndex0())
					- pimpl->vertices.get(e.getVertexIndex1());
			double length = v.length();
			if (length > -epsilon && length < epsilon) {
				continue;
			}
			edgeDirections.emplace_back(v.normalized());
		}

		pimpl->edgeDirections = std::unique_ptr<NormalArray>(
				new NormalArray(edgeDirections));
	}
	return *pimpl->edgeDirections;
}

/*
 * lazy evaluation of edges
 */
const std::vector<ConvexHull::Edge> & ConvexHull::getEdges() const {
	std::lock_guard<std::mutex> locker(pimpl->m_edgesLock);

	if (pimpl->edges == nullptr) {
		pimpl->edges = std::unique_ptr<std::vector<Edge>>(
				new std::vector<Edge>());

		auto n = pimpl->edgeIndices.size();
		pimpl->edges->reserve(n);

		for (size_t i = 0; i < n; i += 4) {
			pimpl->edges->emplace_back(pimpl->edgeIndices.get(i),
					pimpl->edgeIndices.get(i + 1),
					pimpl->edgeIndices.get(i + 2),
					pimpl->edgeIndices.get(i + 3));
		}
	}
	return *pimpl->edges;
}

/**
 * Get unique face directions
 */
const NormalArray & ConvexHull::getFaceDirections() const {
	std::lock_guard<std::mutex> locker(pimpl->m_faceDirectionsLock);

	if (pimpl->faceDirections == nullptr) {
		auto nFaceDirections = pimpl->faceDirectionIndices.size();
		std::vector<Normal> faceDirections;
		faceDirections.reserve(nFaceDirections);

		for (size_t i = 0; i < nFaceDirections; ++i) {
			faceDirections.emplace_back(
					pimpl->faceNormals.get(pimpl->faceDirectionIndices.get(i)));
		}

		pimpl->faceDirections = std::unique_ptr<NormalArray>(
				new NormalArray(faceDirections));
	}
	return *pimpl->faceDirections;
}

/**
 * get face normals of convex hull
 *
 * @return  face normals
 */
const NormalArray & ConvexHull::getFaceNormals() const {
	return pimpl->faceNormals;
}

/**
 * Get planes of convex hull
 */
const std::vector<Plane> & ConvexHull::getPlanes() const {
	std::lock_guard<std::mutex> locker(pimpl->m_planesLock);

	if (pimpl->planes == nullptr) {
		pimpl->planes = std::unique_ptr<std::vector<Plane>>(
				new std::vector<Plane>());

		auto nPlanes = pimpl->planeIndices.size();
		pimpl->planes->reserve(nPlanes);

		for (size_t i = 0; i < nPlanes; ++i) {
			pimpl->planes->emplace_back(
					pimpl->vertices.get(pimpl->planeIndices.get(i)),
					pimpl->faceNormals.get(i));
		}
	}
	return *pimpl->planes;
}

/*
 * lazy evaluation of sphere
 */
const Sphere & ConvexHull::getSphere() const {
	std::lock_guard<std::mutex> locker(pimpl->m_sphereLock);

	if (pimpl->sphere == nullptr) {
		pimpl->sphere = std::unique_ptr<Sphere>(
				new Sphere(pimpl->bounds.getCentre(),
						static_cast<float>(pimpl->bounds.getRadius())));
	}
	return *pimpl->sphere;
}

/*
 * lazy evaluation of triangle list
 */
const TriangleList & ConvexHull::getTriangleList() const {
	std::lock_guard<std::mutex> locker(pimpl->m_triangleListLock);

	if (pimpl->triangleList == nullptr) {
		pimpl->triangleList = std::unique_ptr<TriangleList>(
				new TriangleList(pimpl->indices, pimpl->vertices));
	}
	return *pimpl->triangleList;
}

/**
 * get vertices of convex hull
 *
 * @return  vertices of convex hull
 */
const Vec3Array & ConvexHull::getVertices() const {
	return pimpl->vertices;
}

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
bool ConvexHull::rayIntersection(const Ray & ray, Vec3 & point,
		double & distance) const {
	Vec3 p;
	double d;
	if (pimpl->bounds.rayIntersection(ray, p, d) == false) {
		return false;
	}
	return getTriangleList().rayIntersection(ray, point, distance);
}

/**
 * create version of convex hull transformed by matrix
 *
 * @param m  matrix to transform by
 *
 * @return   transformed convex hull
 */
ConvexHull ConvexHull::transformed(const Mat4 & m) const {
	Mat3 r = m.getRotationScale();
	Vec3Array newVertices = pimpl->vertices.transformed(m);
	NormalArray newFaceNormals = pimpl->faceNormals.transformed(r);

	return ConvexHull(newVertices.getBounds(), newVertices, pimpl->indices,
			newFaceNormals, pimpl->planeIndices, pimpl->edgeIndices,
			pimpl->faceDirectionIndices, pimpl->edgeDirectionIndices);
}

/**
 * create version of convex hull transformed by transform
 *
 * @param transform  transform to transform by
 *
 * @return           transformed convex hull
 */
ConvexHull ConvexHull::transformed(const Transform & transform) const {
	Mat3 r = transform.getRotationMatrix();
	Vec3Array newVertices = pimpl->vertices.transformed(transform);
	NormalArray newFaceNormals = pimpl->faceNormals.transformed(r);

	return ConvexHull(newVertices.getBounds(), newVertices, pimpl->indices,
			newFaceNormals, pimpl->planeIndices, pimpl->edgeIndices,
			pimpl->faceDirectionIndices, pimpl->edgeDirectionIndices);
}

/**
 * check convex hull loaded
 *
 * @return  true, if validation successful, false otherwise
 */
bool ConvexHull::validate() const {
	std::lock_guard<std::mutex> locker(pimpl->m_validateLock);

	if (pimpl->vertices.validate() == false) {
		return false;
	}
	if (pimpl->indices.validate() == false) {
		return false;
	}
	if (pimpl->faceNormals.validate() == false) {
		return false;
	}
	if (pimpl->planeIndices.validate() == false) {
		return false;
	}
	if (pimpl->edgeIndices.validate() == false) {
		return false;
	}
	if (pimpl->faceDirectionIndices.validate() == false) {
		return false;
	}
	if (pimpl->edgeDirectionIndices.validate() == false) {
		return false;
	}
	return true;
}

/**
 * get script object factory for ConvexHull
 *
 * @return  ConvexHull factory
 */
STATIC const ScriptObjectPtr & ConvexHull::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
