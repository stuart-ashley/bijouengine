#pragma once

#include "shadow.h"

#include "../core/abstractSpotlight.h"
#include "../core/transform.h"

namespace render {
	class Spotlight {
	public:
		Spotlight(int id, const AbstractSpotlight & light,
				const Transform & transform);

		Shadow createShadowForAspect(const Aspect & aspect) const;

		inline const Color & getColor() const {
			return light.getColor();
		}

		inline const ConvexHull & getConvexHull() const {
			return light.getConvexHull();
		}

		inline float getDistance() const {
			return light.getDistance();
		}

		inline int getId() const {
			return id;
		}

		/**
		 * get sine of half angle
		 *
		 * @return  sine of half angle
		 */
		float getSineHalfAngle() const;

		inline const Transform & getTransform() const {
			return transform;
		}
	private:
		int id;
		AbstractSpotlight light;
		Transform transform;
	};
}
