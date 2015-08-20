#include "occlusionMap.h"

#include "aspect.h"
#include "convexHull.h"
#include "intersection.h"
#include "normalArray.h"
#include "transform.h"

#include <algorithm>
#include <limits>

namespace {
	struct MinMax {
		std::vector<int> min;
		std::vector<int> max;
		int width;
		int height;
		int miny;
		int maxy;

		/**
		 * Constructor
		 *
		 * @param height
		 */
		MinMax(int width, int height) :
						width(width),
						height(height),
						miny(std::numeric_limits<int>::max()),
						maxy(std::numeric_limits<int>::lowest()) {
			min.reserve(height);
			max.reserve(height);
			for (int i = 0; i < height; ++i) {
				min.emplace_back(std::numeric_limits<int>::max());
				max.emplace_back(std::numeric_limits<int>::lowest());
			}
		}

		/**
		 * Write line to min max
		 *
		 * @param x0
		 * @param y0
		 * @param x1
		 * @param y1
		 */
		void write(int x0, int y0, int x1, int y1) {
			// clip against map edges
			if ((x0 < 0 && x1 < 0) || (y0 < 0 && y1 < 0)
					|| (x0 >= width && x1 >= width)
					|| (y0 >= height && y1 >= height)) {
				return;
			}

			miny = std::min(std::min(miny, y0), y1);
			maxy = std::max(std::max(maxy, y0), y1);

			int dx = std::abs(x1 - x0);
			int dy = std::abs(y1 - y0);
			int sx = x0 < x1 ? 1 : -1;
			int sy = y0 < y1 ? 1 : -1;
			int err = dx - dy;

			while (true) {
				if (y0 >= 0 && y0 < height) {
					min[y0] = std::min(min[y0], x0);
					max[y0] = std::max(max[y0], x0);
				}
				if (x0 == x1 && y0 == y1) {
					break;
				}
				int e2 = 2 * err;
				if (e2 > -dy) {
					err = err - dy;
					x0 = x0 + sx;
				}
				if (e2 < dx) {
					err = err + dx;
					y0 = y0 + sy;
				}
			}
		}
	};

	Mat4 calcProjectionMatrix(const Aspect & aspect, float width,
			float height) {
		Mat4 m(.5f * width, 0.f, 0.f, .5f * width, 0.f, .5f * height, 0.f,
				.5f * height, 0.f, 0.f, .5f, .5f, 0.f, 0.f, 0.f, 1.f);
		m.mul(aspect.getCamera()->getProjectionMatrix());
		return m;
	}
}

struct OcclusionMap::impl {
	std::vector<float> map;
	int width;
	int height;
	double minz;
	Aspect aspect;
	Mat4 cameraToScreen;
	Plane nearPlane;

	impl(const Aspect & aspect, int width, int height) :
					map(width * height, 1),
					width(width),
					height(height),
					minz(1),
					aspect(aspect),
					cameraToScreen(
							calcProjectionMatrix(aspect,
									static_cast<float>(width),
									static_cast<float>(height))),
					nearPlane(aspect.getConvexHull().getPlanes()[0]) {
	}

	/**
	 * Clip edges to plane
	 *
	 * @param edges
	 * @param vertices
	 * @param faces
	 * @param plane
	 */
	void clipEdges(std::vector<ConvexHull::Edge> & edges, Vec3Array & vertices,
			NormalArray & faces, const Plane & plane) {
		auto vsize = vertices.size();

		size_t index = 0;
		while (index < edges.size()) {
			auto & edge = edges[index];

			const auto & a = vertices.get(edge.getVertexIndex0());
			const auto & b = vertices.get(edge.getVertexIndex1());

			double da = plane.distanceTo(a);
			double db = plane.distanceTo(b);

			if (da <= 0) {
				if (db > 0) {
					// a inside, b outside
					Vec3 c;
					c.interpolate(a, b, da / (da - db));
					edge.setVertexIndex1(vertices.size());
					vertices.add(c);
				}
			} else if (db <= 0) {
				// b inside, a outside
				Vec3 c;
				c.interpolate(b, a, db / (db - da));
				edge.setVertexIndex0(vertices.size());
				vertices.add(c);
			} else {
				// a outside, b outside
				edges.erase(edges.begin() + index);
				continue;
			}
			index += 1;
		}

		// test empty
		if (edges.size() == 0) {
			return;
		}

		// test unchanged
		if (vsize == vertices.size()) {
			return;
		}

		//
		// build edges from clip points
		//

		// 2 new vertices, must have clipped a plane
		if (vertices.size() - vsize == 2) {
			edges.emplace_back(vsize, vsize + 1, edges[0].getFaceNormalIndex0(),
					edges[0].getFaceNormalIndex1());
			return;
		}

		// add new face
		auto newFaceIdx = faces.size();
		faces.add(plane.getNormal());

		for (size_t i = vsize; i < vertices.size() - 1; ++i) {
			for (size_t j = i + 1; j < vertices.size(); ++j) {
				auto f = getFaceIndex(i, j, edges);
				if (f != std::numeric_limits<size_t>::max()) {
					edges.emplace_back(i, j, f, newFaceIdx);
				}
			}
		}
	}

	/**
	 * Get element
	 *
	 * @param x
	 * @param y
	 * @return
	 */
	float get(int x, int y) {
		return map[y * width + x];
	}

	/**
	 * Given 2 vertex indices on open edge find the common face
	 *
	 * @param   v0
	 * @param   v1
	 * @param   edges
	 *
	 * @return  common face index, or max size_t if no common face
	 */
	size_t getFaceIndex(size_t v0, size_t v1,
			const std::vector<ConvexHull::Edge> & edges) {
		size_t a0 = 0;
		size_t a1 = 0;
		size_t b0 = 0;
		size_t b1 = 0;
		bool foundFirst = false;
		bool foundSecond = false;
		for (const auto & edge : edges) {
			if (edge.getVertexIndex0() == v0 || edge.getVertexIndex1() == v0) {
				a0 = edge.getFaceNormalIndex0();
				a1 = edge.getFaceNormalIndex1();
				foundFirst = true;
			}
			if (edge.getVertexIndex0() == v1 || edge.getVertexIndex1() == v1) {
				b0 = edge.getFaceNormalIndex0();
				b1 = edge.getFaceNormalIndex1();
				foundSecond = true;
			}
		}
		if (foundFirst == false || foundSecond == false) {
			return std::numeric_limits<size_t>::max();
		}
		if (a0 == b0 || a0 == b1) {
			if (a1 == b0 || a1 == b1) {
				return std::numeric_limits<size_t>::max();
			}
			return a0;
		}
		if (a1 == b0 || a1 == b1) {
			return a1;
		}
		return std::numeric_limits<size_t>::max();
	}

	/**
	 * Test min max against occlusion map
	 *
	 * @param minMax
	 * @param z
	 * @return
	 */
	bool isOccluded(const MinMax & minMax, float z) {
		int miny = std::max(0, minMax.miny);
		int maxy = std::min(height, minMax.maxy + 1);
		for (int y = miny; y < maxy; ++y) {
			int minx = std::max(0, minMax.min[y]);
			int maxx = std::min(width, minMax.max[y] + 1);
			for (int x = minx; x < maxx; ++x) {
				if (z < get(x, y)) {
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * Write element
	 *
	 * @param x
	 * @param y
	 * @param z
	 */
	void put(int x, int y, float z) {
		map[y * width + x] = std::min(map[y * width + x], z);
	}

	/**
	 * Write min max to occlusion map
	 *
	 * @param minMax
	 * @param z
	 */
	void write(const MinMax & minMax, float z) {
		int miny = std::max(0, minMax.miny + 1);
		int maxy = std::min(height, minMax.maxy);
		for (int y = miny; y < maxy; ++y) {
			int minx = std::max(0, minMax.min[y] + 1);
			int maxx = std::min(width, minMax.max[y]);
			for (int x = minx; x < maxx; ++x) {
				put(x, y, z);
			}
		}
	}
};

/**
 * Create new occlusion map
 *
 * @param aspect
 * @param width
 * @param height
 */
OcclusionMap::OcclusionMap(const Aspect & aspect, int width, int height) :
		pimpl(new impl(aspect, width, height)) {
}

/**
 * destructor
 */
OcclusionMap::~OcclusionMap() {
}

/**
 * Write occluder to occlusion map
 *
 * @param aspect
 * @param occluder
 * @param debugGeometry
 */
void OcclusionMap::write(const ConvexHull & occluder) {
	// compare occluder to camera frustum
	Intersection intxn;
	if (occluder.collide(pimpl->aspect.getConvexHull(),
			pimpl->aspect.getRotTrans(), intxn) == false) {
		return;
	}

	// vertices in camera space
	auto worldToCamera = pimpl->aspect.getRotTrans().inverse();
	auto vertices = occluder.getVertices().transformed(worldToCamera);

	// face normals in camera space
	auto w2cRotation = worldToCamera.getRotationMatrix();
	NormalArray normals = occluder.getFaceNormals().transformed(w2cRotation);

	// clip to near plane
	std::vector<ConvexHull::Edge> clippedEdges = occluder.getEdges();
	pimpl->clipEdges(clippedEdges, vertices, normals, pimpl->nearPlane);

	// silhouette edges
	double z = 0;
	std::vector<ConvexHull::Edge> prunedEdges;
	for (const auto & edge : clippedEdges) {
		const auto & v = vertices.get(edge.getVertexIndex0());
		double d0 = v.dot(normals.get(edge.getFaceNormalIndex0()));
		double d1 = v.dot(normals.get(edge.getFaceNormalIndex1()));
		if ((d0 < 0 && d1 > 0) || (d0 > 0 && d1 < 0)) {
			// farthest z
			z = std::max(z, -v.getZ());
			z = std::max(z, -vertices.get(edge.getVertexIndex1()).getZ());
			prunedEdges.emplace_back(edge);
		}
	}
	z /= pimpl->aspect.getCamera()->getFar();
	// keep track of minimum z
	pimpl->minz = std::min(pimpl->minz, z);
	// vertices to screen space
	vertices.transformed(pimpl->cameraToScreen);
	// screen space edges
	MinMax minMax(pimpl->width, pimpl->height);
	for (const auto & edge : prunedEdges) {
		const auto & v0 = vertices.get(edge.getVertexIndex0());
		const auto & v1 = vertices.get(edge.getVertexIndex1());
		minMax.write(static_cast<int>(v0.getX()), static_cast<int>(v0.getY()),
				static_cast<int>(v1.getX()), static_cast<int>(v1.getY()));
	}
	pimpl->write(minMax, static_cast<float>(z));
}

/**
 * Test occluder against occlusion map
 *
 * @param aspect
 * @param toWorld
 * @param occluder
 * @return
 */
bool OcclusionMap::isOccluded(const Transform & toWorld,
		const ConvexHull & occluder) const {
	// vertices in camera space
	auto toCamera = toWorld.to(pimpl->aspect.getRotTrans());
	auto vertices = occluder.getVertices().transformed(toCamera);
	// compare occluder to near clip plane
	if (pimpl->nearPlane.isAbove(vertices) == false) {
		return false;
	}
	// nearest z
	float far = pimpl->aspect.getCamera()->getFar();
	double z = far;
	for (const auto & v : vertices) {
		z = std::min(-v.getZ(), z);
	}
	z /= far;
	// check against minimum z
	if (z < pimpl->minz) {
		return false;
	}
	// vertices to screen space
	vertices = vertices.transformed(pimpl->cameraToScreen);
	MinMax minMax(pimpl->width, pimpl->height);
	// screen space edges
	for (const auto & edge : occluder.getEdges()) {
		const auto & v0 = vertices.get(edge.getVertexIndex0());
		const auto & v1 = vertices.get(edge.getVertexIndex1());
		minMax.write(static_cast<int>(v0.getX()), static_cast<int>(v0.getY()),
				static_cast<int>(v1.getX()), static_cast<int>(v1.getY()));
	}
	return pimpl->isOccluded(minMax, static_cast<float>(z));
}
