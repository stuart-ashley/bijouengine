#pragma once

#include <memory>

class Aspect;
class Mat3;
class Mat4;

namespace render {
	class Texture;
	class View;

	class Shadow {
	public:

		Shadow(const std::shared_ptr<Texture> & texture,
				const std::shared_ptr<View> & view, const Aspect & shadowAspect,
				const Aspect & viewerAspect);

		/**
		 * get shadow texture
		 *
		 * @return  shadow texture
		 */
		const std::shared_ptr<Texture> & getTexture() const;

		/**
		 * get texture matrix
		 *
		 * @return  texture matrix
		 */
		const Mat4 & getTextureMatrix() const;

		/**
		 * get view for shadow
		 *
		 * @return  view for shadow
		 */
		const std::shared_ptr<View> & getView() const;

		/**
		 * get view to light matrix
		 *
		 * @return  view to light matrix
		 */
		const Mat3 & getViewToLight() const;

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

