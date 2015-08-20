#pragma once

#include "shadow.h"
#include "view.h"

#include "../core/abstractSunlight.h"
#include "../core/transform.h"

#include <vector>

namespace render {
	class Sunlight {
	public:
		Sunlight(int id, const AbstractSunlight & light,
				const Transform & transform);

		std::vector<Shadow> createShadowsForAspect(const Aspect & aspect,
				const View::ModifierSet & modifiers) const;

		/**
		 * get light color
		 *
		 * @return  color of sunlight
		 */
		inline const Color & getColor() const {
			return light.getColor();
		}

		/**
		 * get light direction
		 *
		 * @return  direction of sunlight
		 */
		inline const Normal & getDirection() const {
			return direction;
		}

		inline int getId() const {
			return id;
		}

		/**
		 * get light transform
		 *
		 * @return  transform of sunlight
		 */
		inline const Transform & getTransform() const {
			return transform;
		}
	private:
		int id;
		AbstractSunlight light;
		Transform transform;
		/** world space sunlight direction */
		Normal direction;
	};
}
