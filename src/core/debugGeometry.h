#pragma once

#include <memory>

class BoundingBox;
class Color;
class Transform;
class TriangleList;
class Vec3;

#include <vector>

class DebugGeometry {
public:

	DebugGeometry(const Transform & transform, const Color & color,
			const TriangleList & triangles);

	DebugGeometry(const Transform & transform, const Color & color,
			const BoundingBox & box);

	DebugGeometry(const Transform & transform, const Color & color,
			const Color & color2, const Vec3 & p0, const Vec3 & p1);

	DebugGeometry(const Transform & transform, const Color & color,
			const Vec3 & point);

	~DebugGeometry();

	/**
	 * get start color for lines, color for points
	 *
	 * @return  start color for lines, color for points
	 */
	const Color & getColor() const;

	/**
	 * get end color for lines
	 *
	 * @return  end color for lines
	 */
	const Color & getEndColor() const;

	/**
	 * get line elements of debug
	 *
	 * @return  list of points representing start and end of lines
	 */
	const std::vector<Vec3> & getLines() const;

	/**
	 * get point elements of debug
	 *
	 * @return  list of points
	 */
	const std::vector<Vec3> & getPoints() const;

	/**
	 * get transform for debug geometry
	 *
	 * @return  transform
	 */
	const Transform & getTransform() const;

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

