#pragma once

#include "normal.h"
#include "vec3.h"

#include <limits>

class Transform;

class Intersection {
public:

	inline Intersection() :
			m_normal(1.f, 0.f, 0.f), m_depth(std::numeric_limits<double>::max()) {
	}

	inline Intersection(const Vec3 & point, const Normal & normal, double depth) :
			m_point(point), m_normal(normal), m_depth(depth) {
	}

	inline void flipNormal() {
		m_normal = -m_normal;
	}

	inline double getDepth() const {
		return m_depth;
	}

	inline const Normal & getNormal() const {
		return m_normal;
	}

	inline const Vec3 & getPoint() const {
		return m_point;
	}

	inline void set(const Vec3 & point, const Normal & normal, double depth) {
		m_point = point;
		m_normal = normal;
		m_depth = depth;
	}

	inline std::string toString() const {
		return "Intersection(" + m_point.toString() + "," + m_normal.toString()
				+ "," + std::to_string(m_depth) + ")";
	}

	void transform(const Transform & rotTrans);

private:
	Vec3 m_point;
	Normal m_normal;
	double m_depth;
};

