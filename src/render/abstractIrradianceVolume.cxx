#include "abstractIrradianceVolume.h"

#include "texture.h"

#include "../core/boundingBox.h"
#include "../core/normal.h"
#include "../core/vec3.h"

#include <cmath>
#include <iostream>

using namespace render;

struct AbstractIrradianceVolume::impl {
	int sizex;
	int sizey;
	int sizez;
	const unsigned char * buffer;
	std::array<std::shared_ptr<Texture>, 7> light;
	BoundingBox bounds;
	Vec3 extent;

	impl(int sizex, int sizey, int sizez, const unsigned char * buffer,
			const std::array<std::shared_ptr<Texture>, 7> & light,
			const BoundingBox & bounds) :
					sizex(sizex),
					sizey(sizey),
					sizez(sizez),
					buffer(buffer),
					light(light),
					bounds(bounds),
					extent(bounds.getExtents()) {
	}

	/**
	 *
	 */
	double getBufferValue(int idx, double scale) {
		return (buffer[idx] / 255.0 - .5) * scale;
	}

	/**
	 *
	 */
	Vec3 getValue(int x, int y, int z, float nx, float ny, float nz) {
		float K0 = 0.8862269254527579f; // sqrt( pi / 4 )
		float K1 = 1.0233267079464885f; // sqrt( pi / 3 )
		float K2 = 0.8580855308097833f; // sqrt( 15 * pi / 64 )
		float K3 = 0.2477079561003757f; // sqrt( 5 * pi / 256 )
		float K4 = 0.42904276540489167f; // sqrt( 15 * pi / 256 )

		int idx = (((6 * sizez + z) * sizey + y) * sizex + x) * 4 + 3;
		double exp = buffer[idx] - 127;
		double scale = std::pow(2, exp + 1);

		idx = ((z * sizey + y) * sizex + x) * 4;
		double r = getBufferValue(idx, scale) * K0;
		r += getBufferValue(idx + 1, scale) * ny * K1;
		r += getBufferValue(idx + 2, scale) * nz * K1;
		r += getBufferValue(idx + 3, scale) * nx * K1;

		idx = (((sizez + z) * sizey + y) * sizex + x) * 4;
		double g = getBufferValue(idx, scale) * K0;
		g += getBufferValue(idx + 1, scale) * ny * K1;
		g += getBufferValue(idx + 2, scale) * nz * K1;
		g += getBufferValue(idx + 3, scale) * nx * K1;

		idx = (((2 * sizez + z) * sizey + y) * sizex + x) * 4;
		double b = getBufferValue(idx, scale) * K0;
		b += getBufferValue(idx + 1, scale) * ny * K1;
		b += getBufferValue(idx + 2, scale) * nz * K1;
		b += getBufferValue(idx + 3, scale) * nx * K1;

		idx = (((3 * sizez + z) * sizey + y) * sizex + x) * 4;
		r += getBufferValue(idx, scale) * K2 * nx * ny;
		g += getBufferValue(idx + 1, scale) * K2 * nx * ny;
		b += getBufferValue(idx + 2, scale) * K2 * nx * ny;
		r += getBufferValue(idx + 3, scale) * K4 * (nx * nx - ny * ny);

		idx = (((4 * sizez + z) * sizey + y) * sizex + x) * 4;
		r += getBufferValue(idx, scale) * K2 * ny * nz;
		g += getBufferValue(idx + 1, scale) * K2 * ny * nz;
		b += getBufferValue(idx + 2, scale) * K2 * ny * nz;
		g += getBufferValue(idx + 3, scale) * K4 * (nx * nx - ny * ny);

		idx = (((5 * sizez + z) * sizey + y) * sizex + x) * 4;
		r += getBufferValue(idx, scale) * K3 * (3 * nz * nz - 1);
		g += getBufferValue(idx + 1, scale) * K3 * (3 * nz * nz - 1);
		b += getBufferValue(idx + 2, scale) * K3 * (3 * nz * nz - 1);
		b += getBufferValue(idx + 3, scale) * K4 * (nx * nx - ny * ny);

		idx = (((6 * sizez + z) * sizey + y) * sizex + x) * 4;
		r += getBufferValue(idx, scale) * K2 * nz * nx;
		g += getBufferValue(idx + 1, scale) * K2 * nz * nx;
		b += getBufferValue(idx + 2, scale) * K2 * nz * nx;

		return Vec3(r, g, b);
	}
};

AbstractIrradianceVolume::AbstractIrradianceVolume(int sizex, int sizey,
		int sizez, const unsigned char * buffer,
		const std::array<std::shared_ptr<Texture>, 7> & light,
		const BoundingBox & bounds) :
		pimpl(new impl(sizex, sizey, sizez, buffer, light, bounds)) {
}

/**
 * destructor
 */
AbstractIrradianceVolume::~AbstractIrradianceVolume() {
}

/**
 *
 */
void AbstractIrradianceVolume::dumpSh(int x, int y, int z) {
	int idx = (((6 * pimpl->sizez + z) * pimpl->sizey + y) * pimpl->sizex + x)
			* 4 + 3;
	double exp = (pimpl->buffer[idx] & 0xff) - 127;
	double scale = std::pow(2, exp + 1);

	for (int i = 0; i < 6; ++i) {
		idx = (((i * pimpl->sizez + z) * pimpl->sizey + y) * pimpl->sizex + x)
				* 4;
		std::cout << pimpl->getBufferValue(idx, scale) << std::endl;
		std::cout << pimpl->getBufferValue(idx + 1, scale) << std::endl;
		std::cout << pimpl->getBufferValue(idx + 2, scale) << std::endl;
		std::cout << pimpl->getBufferValue(idx + 3, scale) << std::endl;
	}

	idx = (((6 * pimpl->sizez + z) * pimpl->sizey + y) * pimpl->sizex + x) * 4;
	std::cout << pimpl->getBufferValue(idx, scale) << std::endl;
	std::cout << pimpl->getBufferValue(idx + 1, scale) << std::endl;
	std::cout << pimpl->getBufferValue(idx + 2, scale) << std::endl;
}

/**
 * get volume bounds
 *
 * @return  volume bounds
 */
const BoundingBox & AbstractIrradianceVolume::getBounds() const {
	return pimpl->bounds;
}

/**
 * get volume buffer
 *
 * @return  volume buffer
 */
const unsigned char * AbstractIrradianceVolume::getBuffer() const{
	return pimpl->buffer;
}

/**
 * get volume extent
 *
 * @return  volume extent
 */
const Vec3 & AbstractIrradianceVolume::getExtent() const {
	return pimpl->extent;
}

/**
 * get sample at point with given direction
 *
 * @param point   point in irradiance volume space
 * @param normal  normal in irradiance volume space
 *
 * @return        color for point, normal
 */
Color AbstractIrradianceVolume::getSample(const Vec3 & point,
		const Normal & normal) {
	double x = std::max(.0,
			(point.getX() / pimpl->extent.getX() + .5) * pimpl->sizex - .5f);
	double y = std::max(.0,
			(point.getY() / pimpl->extent.getY() + .5) * pimpl->sizey - .5f);
	double z = std::max(.0,
			(point.getZ() / pimpl->extent.getZ() + .5) * pimpl->sizez - .5f);

	int x0 = (int) x;
	int y0 = (int) y;
	int z0 = (int) z;

	double fx = x - x0;
	double fy = y - y0;
	double fz = z - z0;

	x0 = std::min(pimpl->sizex - 1, x0);
	y0 = std::min(pimpl->sizey - 1, y0);
	z0 = std::min(pimpl->sizez - 1, z0);

	int x1 = std::min(pimpl->sizex - 1, x0 + 1);
	int y1 = std::min(pimpl->sizey - 1, y0 + 1);
	int z1 = std::min(pimpl->sizez - 1, z0 + 1);

	Vec3 v000 = pimpl->getValue(x0, y0, z0, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v100 = pimpl->getValue(x1, y0, z0, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v010 = pimpl->getValue(x0, y1, z0, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v110 = pimpl->getValue(x1, y1, z0, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v001 = pimpl->getValue(x0, y0, z1, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v101 = pimpl->getValue(x1, y0, z1, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v011 = pimpl->getValue(x0, y1, z1, normal.getX(), normal.getY(),
			normal.getZ());
	Vec3 v111 = pimpl->getValue(x1, y1, z1, normal.getX(), normal.getY(),
			normal.getZ());

	Vec3 vx00;
	vx00.interpolate(v000, v100, fx);
	Vec3 vx10;
	vx10.interpolate(v010, v110, fx);
	Vec3 vx01;
	vx01.interpolate(v001, v101, fx);
	Vec3 vx11;
	vx11.interpolate(v011, v111, fx);

	Vec3 vxy0;
	vxy0.interpolate(vx00, vx10, fy);
	Vec3 vxy1;
	vxy1.interpolate(vx01, vx11, fy);

	Vec3 vxyz;
	vxyz.interpolate(vxy0, vxy1, fz);

	return Color(static_cast<float>(vxyz.getX()),
			static_cast<float>(vxyz.getY()), static_cast<float>(vxyz.getZ()));
}

/**
 * get size in x axis
 *
 * @return  size x
 */
int AbstractIrradianceVolume::getSizeX() const {
	return pimpl->sizex;
}

/**
 * get size in y axis
 *
 * @return  size y
 */
int AbstractIrradianceVolume::getSizeY() const {
	return pimpl->sizey;
}

/**
 * get size in z axis
 *
 * @return  size z
 */
int AbstractIrradianceVolume::getSizeZ() const {
	return pimpl->sizez;
}

/**
 * get irradiance volume textures
 *
 * @return  irradiance volume textures
 */
const std::array<std::shared_ptr<Texture>, 7> & AbstractIrradianceVolume::getTextures() const {
	return pimpl->light;
}

bool AbstractIrradianceVolume::validate() {
	for (const auto & t : pimpl->light) {
		if (t->isEnabled() == false) {
			return false;
		}
	}
	return true;
}
