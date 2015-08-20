#pragma once

#include "../core/color.h"

#include <array>
#include <memory>

class BoundingBox;
class Normal;
class Vec3;

namespace render {
	class Texture;

	class AbstractIrradianceVolume {
	public:

		AbstractIrradianceVolume(int sizex, int sizey, int sizez,
				const unsigned char * buffer,
				const std::array<std::shared_ptr<Texture>, 7> & light,
				const BoundingBox & bounds);

		/**
		 * destructor
		 */
		~AbstractIrradianceVolume();

		/**
		 *
		 */
		void dumpSh(int x, int y, int z);

		/**
		 * get volume bounds
		 *
		 * @return  volume bounds
		 */
		const BoundingBox & getBounds() const;

		/**
		 * get volume buffer
		 *
		 * @return  volume buffer
		 */
		const unsigned char * getBuffer() const;

		/**
		 * get volume extent
		 *
		 * @return  volume extent
		 */
		const Vec3 & getExtent() const;

		/**
		 * get sample at point with given direction
		 *
		 * @param point   point in irradiance volume space
		 * @param normal  normal in irradiance volume space
		 *
		 * @return        color for point, normal
		 */
		Color getSample(const Vec3 & point, const Normal & normal);

		/**
		 * get size in x axis
		 *
		 * @return  size x
		 */
		int getSizeX() const;

		/**
		 * get size in y axis
		 *
		 * @return  size y
		 */
		int getSizeY() const;

		/**
		 * get size in z axis
		 *
		 * @return  size z
		 */
		int getSizeZ() const;

		/**
		 * get irradiance volume textures
		 *
		 * @return  irradiance volume textures
		 */
		const std::array<std::shared_ptr<Texture>, 7> & getTextures() const;

		bool validate();

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

