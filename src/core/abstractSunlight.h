#pragma once

#include "abstractLight.h"
#include "vec2.h"

class AbstractSunlight: public AbstractLight {
public:

	inline AbstractSunlight(const std::string & name, const Color & color) :
			AbstractLight(name, color), m_hasExtent(false) {
	}

	inline AbstractSunlight(const std::string & name, const Color & color,
			const Vec2 & extent) :
			AbstractLight(name, color), m_hasExtent(true), m_extent(extent) {
	}

	inline bool hasExtent() const {
		return m_hasExtent;
	}

	inline const Vec2 & getExtent() const {
		return m_extent;
	}

private:
	bool m_hasExtent;
	Vec2 m_extent;
};

