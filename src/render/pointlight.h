#pragma once

#include "../core/abstractLight.h"

namespace render {
	class Pointlight: public AbstractLight {
	public:
		inline Pointlight(const AbstractLight & light, const Vec3 & position) :
				AbstractLight(light), m_position(position) {
		}

		inline const Vec3 & getPosition() const {
			return m_position;
		}
	private:
		Vec3 m_position;
	};
}

