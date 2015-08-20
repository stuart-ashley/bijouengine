#pragma once

#include "renderTask.h"

#include <array>
#include <memory>

namespace render {
	class SphericalHarmonics final: public RenderTask {
	public:

		/**
		 *
		 * @param name
		 * @param texture
		 * @param floats
		 */
		SphericalHarmonics(const std::string & name,
				const std::shared_ptr<Texture> & texture,
				const std::array<float, 27> & floats);

		/**
		 * default destructor
		 */
		inline virtual ~SphericalHarmonics() = default;

		/**
		 *
		 */
		void execute(Canvas & canvas) override;

		/**
		 *
		 * @return
		 */
		bool hasExecuted() const override;

		/**
		 *
		 */
		static void init();

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

