#include "vec3Array.h"

#include "binaryFile.h"
#include "binaryFileCache.h"
#include "boundingBox.h"
#include "mat4.h"
#include "transform.h"
#include "vec3.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <limits>
#include <vector>

namespace {
	/*
	 *
	 */
	std::vector<BaseParameter> params = { Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr), Parameter<Real>("count",
					nullptr) };

	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			if (typeid(*stack.top()) == typeid(List)) {
				auto list = getArg<List>("list", stack, 1);

				std::vector<Vec3> vertices;

				for (const auto & e : list) {
					scriptExecutionAssertType<Vec3>(e, "Require vertex list");

					vertices.emplace_back(*std::static_pointer_cast<Vec3>(e));
				}

				stack.emplace(std::make_shared<Vec3Array>(vertices));

				return;

			}
			auto args = parameters.getArgs(nArgs, stack);

			auto file = std::static_pointer_cast<String>(args["file"]);
			auto offset = std::static_pointer_cast<Real>(args["offset"]);
			auto count = std::static_pointer_cast<Real>(args["count"]);

			scriptExecutionAssert(offset->isInt32(), "Require integer offset");

			scriptExecutionAssert(count->isInt32(), "Require integer count");

			auto binFile = BinaryFileCache::get(currentDir, file->getValue());

			stack.emplace(
					std::make_shared<Vec3Array>(binFile, offset->getInt32(),
							count->getInt32()));
		}
	};
}

struct Vec3Array::impl {
	std::vector<Vec3> m_vertices;
	std::unique_ptr<BoundingBox> m_bounds;
	std::shared_ptr<BinaryFile> m_binaryFile;
	int m_offset;
	int m_count;
	std::atomic<bool> m_valid;

	/*
	 *
	 */
	impl(const std::vector<Vec3> & vertices) :
			m_vertices(vertices), m_offset(0), m_count(0), m_valid(true) {
	}

	/*
	 *
	 */
	impl(const std::shared_ptr<BinaryFile> & binaryFile, int offset, int count) :
					m_binaryFile(std::move(binaryFile)),
					m_offset(offset),
					m_count(count),
					m_valid(false) {
	}

	/*
	 *
	 */
	bool validate() {
		if (m_valid) {
			return true;
		}
		if (m_binaryFile->valid()) {
			m_vertices.reserve(m_count);

			const float * floats =
					reinterpret_cast<const float *>(m_binaryFile->getData()
							+ m_offset);

			for (int i = 0; i < m_count; ++i) {
				double x = *floats++;
				double y = *floats++;
				double z = *floats++;
				m_vertices.emplace_back(x, y, z);
			}

			// no longer needed
			m_binaryFile = nullptr;

			m_valid = true;
			return true;
		}
		return false;
	}
};

/*
 *
 */
Vec3Array::Vec3Array(const std::initializer_list<Vec3> & vertices) :
		pimpl(std::make_shared<impl>(vertices)) {
}

/*
 *
 */
Vec3Array::Vec3Array(const std::vector<Vec3> & vertices) :
		pimpl(std::make_shared<impl>(vertices)) {
}

/*
 *
 */
Vec3Array::Vec3Array(const std::shared_ptr<BinaryFile> & binaryFile, int offset,
		int count) :
		pimpl(std::make_shared<impl>(binaryFile, offset, count)) {
}

/*
 *
 */
Vec3Array::~Vec3Array() {
}

/**
 * add vertex to array
 *
 * @param v  vertex to add
 */
void Vec3Array::add(const Vec3 & v) {
	pimpl->m_vertices.emplace_back(v);
}

/*
 *
 */
std::vector<Vec3>::const_iterator Vec3Array::begin() const {
	return pimpl->m_vertices.cbegin();
}

/*
 *
 */
std::vector<Vec3>::const_iterator Vec3Array::end() const {
	return pimpl->m_vertices.cend();
}

/*
 *
 */
const Vec3 & Vec3Array::get(size_t idx) const {
	return pimpl->m_vertices[idx];
}

/**
 * calculate bounds
 *
 * @return  bounds
 */
const BoundingBox & Vec3Array::getBounds() const {
	if (pimpl->m_bounds == nullptr && pimpl->m_vertices.size() > 0) {
		const auto & first = pimpl->m_vertices[0];

		double minx = first.getX();
		double miny = first.getY();
		double minz = first.getZ();

		double maxx = first.getX();
		double maxy = first.getY();
		double maxz = first.getZ();

		for (const auto & v : pimpl->m_vertices) {
			minx = std::min(minx, v.getX());
			miny = std::min(miny, v.getY());
			minz = std::min(minz, v.getZ());

			maxx = std::max(maxx, v.getX());
			maxy = std::max(maxy, v.getY());
			maxz = std::max(maxz, v.getZ());
		}
		pimpl->m_bounds = std::unique_ptr<BoundingBox>(
				new BoundingBox(Vec3(minx, miny, minz),
						Vec3(maxx, maxy, maxz)));
	}
	return *pimpl->m_bounds;
}

/**
 * minimum and maximum distance in given direction
 *
 * @param direction  direction to get values for
 * @param minDist    minimum distance in direction
 * @param maxDist    maximum distance in direction
 */
void Vec3Array::getDirectionMinMax(const Normal & direction, double & minDist,
		double & maxDist) const {
	minDist = std::numeric_limits<double>::max();
	maxDist = -std::numeric_limits<double>::max();

	for (const auto & v : pimpl->m_vertices) {
		double d = v.dot(direction);

		minDist = std::min(minDist, d);
		maxDist = std::max(maxDist, d);
	}
}

/**
 * minimum and maximum point and distance in given direction
 *
 * @param direction  direction to get values for
 * @param minPoint   minimum point in direction
 * @param maxPoint   maximum point in direction
 * @param minDist    minimum distance in direction
 * @param maxDist    maximum distance in direction
 */
void Vec3Array::getDirectionMinMax(const Normal & direction, Vec3 & minPoint,
		Vec3 & maxPoint, double & minDist, double & maxDist) const {
	minDist = std::numeric_limits<double>::max();
	maxDist = -std::numeric_limits<double>::max();

	for (const auto & v : pimpl->m_vertices) {
		double d = v.dot(direction);

		if (d < minDist) {
			minDist = d;
			minPoint = v;
		}
		if (d > maxDist) {
			maxDist = d;
			maxPoint = v;
		}
	}
}

/**
 * Produce a rotated version of this array
 *
 * @param rotation  amount to rotate
 * @return          rotated array
 */
Vec3Array Vec3Array::rotated(const Quat & rotation) const {
	std::vector<Vec3> a;
	a.reserve(pimpl->m_vertices.size());

	Mat3 r(rotation);

	for (const auto & v : pimpl->m_vertices) {
		double x = r.get(0, 0) * v.getX() + r.get(0, 1) * v.getY()
				+ r.get(0, 2) * v.getZ();
		double y = r.get(1, 0) * v.getX() + r.get(1, 1) * v.getY()
				+ r.get(1, 2) * v.getZ();
		double z = r.get(2, 0) * v.getX() + r.get(2, 1) * v.getY()
				+ r.get(2, 2) * v.getZ();
		a.emplace_back(x, y, z);
	}

	return Vec3Array(a);
}

/**
 * produce scaled version of vertices
 *
 * @param s  scale factor
 *
 * @return   scaled vertices
 */
Vec3Array Vec3Array::scaled(float s) const {
	std::vector<Vec3> a;
	a.reserve(pimpl->m_vertices.size());

	for (const auto & point : pimpl->m_vertices) {
		a.emplace_back(point * s);
	}

	return Vec3Array(a);
}

/**
 * get number of vertices in array
 *
 * @return  number of vertices
 */
size_t Vec3Array::size() const {
	return pimpl->m_vertices.size();
}

/**
 *
 * @return
 */
OVERRIDE std::string Vec3Array::toString() const {
	std::string str = "Vec3Array(";

	bool first = false;

	for (const auto & v : pimpl->m_vertices) {
		if (first == false) {
			str += ",\n";
		}
		str += "\t" + v.toString();
		first = false;
	}

	return str + ")";
}

/**
 * Produce transformed version of vertices by matrix
 *
 * @param matrix  transformation matrix
 *
 * @return        transformed vertices
 */
Vec3Array Vec3Array::transformed(const Mat4 & m) const {
	std::vector<Vec3> a;
	a.reserve(pimpl->m_vertices.size());

	for (const auto & point : pimpl->m_vertices) {
		Vec3 p = point;
		m.transform(p);
		a.emplace_back(p);
	}

	return Vec3Array(a);
}

/**
 * Produce transformed version of vertices by transform
 *
 * @param transform  transformation
 *
 * @return           transformed vertices
 */
Vec3Array Vec3Array::transformed(const Transform & transform) const {
	Mat3 r = transform.getRotationMatrix();
	Vec3 t = transform.getTranslation();

	std::vector<Vec3> a;
	a.reserve(pimpl->m_vertices.size());

	for (const auto & v : pimpl->m_vertices) {
		double x = r.get(0, 0) * v.getX() + r.get(0, 1) * v.getY()
				+ r.get(0, 2) * v.getZ() + t.getX();
		double y = r.get(1, 0) * v.getX() + r.get(1, 1) * v.getY()
				+ r.get(1, 2) * v.getZ() + t.getY();
		double z = r.get(2, 0) * v.getX() + r.get(2, 1) * v.getY()
				+ r.get(2, 2) * v.getZ() + t.getZ();

		a.emplace_back(x, y, z);
	}
	return Vec3Array(a);
}

/**
 * If the vertices are from a file make sure it's loaded
 *
 * @return  true if loaded, false otherwise
 */

bool Vec3Array::validate() const {
	return pimpl->validate();
}

/**
 * get script object factory for Vec3Array
 *
 * @param currentDir  current directory of caller
 *
 * @return            Vec3Array factory
 */
STATIC ScriptObjectPtr Vec3Array::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
