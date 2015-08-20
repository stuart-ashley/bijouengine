#pragma once

#include "blendFlag.h"
#include "renderTask.h"

namespace render {
	class Shader;
	class Texture;
	class UniformArray;

	class Panel: public RenderTask {
	public:
		/**
		 *
		 * @param name
		 * @param srcRect
		 * @param dstRect
		 * @param uniforms
		 * @param shader
		 * @param texture
		 * @param source
		 * @param blend
		 * @param level
		 * @param color
		 * @param alpha
		 * @param opaque
		 * @param bgColor
		 */
		Panel(const std::string & name, const Rect & srcRect,
				const Rect & dstRect, const UniformArray & uniforms,
				const std::shared_ptr<Shader> & shader,
				const std::shared_ptr<Texture> & texture,
				const std::shared_ptr<Texture> & source, BlendFlag blend,
				int level, const Color & color, float alpha, bool opaque,
				const Color & bgColor);

		/**
		 * destructor
		 */
		~Panel();

		/**
		 *
		 */
		void execute(Canvas & canvas) override;

		/**
		 *
		 * @return
		 */
		bool hasExecuted() const override;

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

