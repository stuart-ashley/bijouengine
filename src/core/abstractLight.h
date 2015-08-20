#pragma once

#include "color.h"
#include "vec3.h"

class AbstractLight {
public:
	inline AbstractLight(const std::string & name, const Color & color) :
			m_name(name), m_color(color) {
	}

	inline const Color & getColor() const {
		return m_color;
	}

	inline const std::string & getName() const {
		return m_name;
	}

private:
	std::string m_name;
	Color m_color;
};

