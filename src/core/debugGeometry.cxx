#include "debugGeometry.h"

#include "boundingBox.h"
#include "color.h"
#include "transform.h"
#include "triangleList.h"

/*
 *
 */
struct DebugGeometry::impl {
	Transform m_transform;
	Color m_color;
	Color m_color2;
	std::vector<Vec3> m_lines;
	std::vector<Vec3> m_points;

	impl(const Transform & transform, const Color & color, const Color & color2) :
			m_transform(transform), m_color(color), m_color2(color2) {
	}
};

/*
 *
 */
DebugGeometry::DebugGeometry(const Transform & transform, const Color & color,
		const TriangleList & triangles) :
		pimpl(new impl(transform, color, color)) {
	for (const auto & tri : triangles) {
		pimpl->m_lines.emplace_back(tri.getVertex0());
		pimpl->m_lines.emplace_back(tri.getVertex1());
		pimpl->m_lines.emplace_back(tri.getVertex1());
		pimpl->m_lines.emplace_back(tri.getVertex2());
		pimpl->m_lines.emplace_back(tri.getVertex2());
		pimpl->m_lines.emplace_back(tri.getVertex0());

		pimpl->m_points.emplace_back(tri.getCentre());
	}
}

/*
 *
 */
DebugGeometry::DebugGeometry(const Transform & transform, const Color & color,
		const BoundingBox & box) :
		pimpl(new impl(transform, color, color)) {
	const auto & min = box.getMin();
	const auto & max = box.getMax();

	pimpl->m_lines.emplace_back(min.getX(), min.getY(), min.getZ());
	pimpl->m_lines.emplace_back(max.getX(), min.getY(), min.getZ());
	pimpl->m_lines.emplace_back(min.getX(), min.getY(), max.getZ());
	pimpl->m_lines.emplace_back(max.getX(), min.getY(), max.getZ());

	pimpl->m_lines.emplace_back(min.getX(), max.getY(), min.getZ());
	pimpl->m_lines.emplace_back(max.getX(), max.getY(), min.getZ());
	pimpl->m_lines.emplace_back(min.getX(), max.getY(), max.getZ());
	pimpl->m_lines.emplace_back(max.getX(), max.getY(), max.getZ());

	pimpl->m_lines.emplace_back(min.getX(), min.getY(), min.getZ());
	pimpl->m_lines.emplace_back(min.getX(), max.getY(), min.getZ());
	pimpl->m_lines.emplace_back(max.getX(), min.getY(), min.getZ());
	pimpl->m_lines.emplace_back(max.getX(), max.getY(), min.getZ());

	pimpl->m_lines.emplace_back(min.getX(), min.getY(), max.getZ());
	pimpl->m_lines.emplace_back(min.getX(), max.getY(), max.getZ());
	pimpl->m_lines.emplace_back(max.getX(), min.getY(), max.getZ());
	pimpl->m_lines.emplace_back(max.getX(), max.getY(), max.getZ());

	pimpl->m_lines.emplace_back(min.getX(), min.getY(), min.getZ());
	pimpl->m_lines.emplace_back(min.getX(), min.getY(), max.getZ());
	pimpl->m_lines.emplace_back(min.getX(), max.getY(), min.getZ());
	pimpl->m_lines.emplace_back(min.getX(), max.getY(), max.getZ());

	pimpl->m_lines.emplace_back(max.getX(), min.getY(), min.getZ());
	pimpl->m_lines.emplace_back(max.getX(), min.getY(), max.getZ());
	pimpl->m_lines.emplace_back(max.getX(), max.getY(), min.getZ());
	pimpl->m_lines.emplace_back(max.getX(), max.getY(), max.getZ());
}

/*
 *
 */
DebugGeometry::DebugGeometry(const Transform & transform, const Color & color,
		const Color & color2, const Vec3 & p0, const Vec3 & p1) :
		pimpl(new impl(transform, color, color2)) {
	pimpl->m_lines.emplace_back(p0);
	pimpl->m_lines.emplace_back(p1);
}

/*
 *
 */
DebugGeometry::DebugGeometry(const Transform & transform, const Color & color,
		const Vec3 & point) :
		pimpl(new impl(transform, color, color)) {
	pimpl->m_lines.emplace_back(point.getX() + .1f, point.getY(), point.getZ());
	pimpl->m_lines.emplace_back(point.getX() - .1f, point.getY(), point.getZ());
	pimpl->m_lines.emplace_back(point.getX(), point.getY() + .1f, point.getZ());
	pimpl->m_lines.emplace_back(point.getX(), point.getY() - .1f, point.getZ());
	pimpl->m_lines.emplace_back(point.getX(), point.getY(), point.getZ() + .1f);
	pimpl->m_lines.emplace_back(point.getX(), point.getY(), point.getZ() - .1f);
}

/*
 *
 */
DebugGeometry::~DebugGeometry() {
}

/**
 * get end color for lines
 *
 * @return  end color for lines
 */
const Color & DebugGeometry::getEndColor() const {
	return pimpl->m_color2;
}

/**
 * get line elements of debug
 *
 * @return  list of points representing start and end of lines
 */
const std::vector<Vec3> & DebugGeometry::getLines() const {
	return pimpl->m_lines;
}

/**
 * get point elements of debug
 *
 * @return  list of points
 */
const std::vector<Vec3> & DebugGeometry::getPoints() const {
	return pimpl->m_points;
}

/**
 * get start color for lines, color for points
 *
 * @return  start color for lines, color for points
 */
const Color & DebugGeometry::getColor() const {
	return pimpl->m_color;
}

/**
 * get transform for debug geometry
 *
 * @return  transform
 */
const Transform & DebugGeometry::getTransform() const {
	return pimpl->m_transform;
}
