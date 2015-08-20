#pragma once

#include "abstractIrradianceVolume.h"

#include "../core/transform.h"

namespace render {
	class IrradianceVolume {
	public:
		IrradianceVolume(const AbstractIrradianceVolume & volume,
				const Transform & transform) :
				volume(volume), transform(transform) {
		}

		inline const Transform & getTransform() const {
			return transform;
		}

		inline const AbstractIrradianceVolume & getVolume() const {
			return volume;
		}
	private:
		AbstractIrradianceVolume volume;
		Transform transform;
	};
}

