#pragma once

#include <algorithm>
#include <vector>

template<typename TYPE>
class Bucket3d {
public:

	Bucket3d(const BoundingBox & bounds, double minBucketSize) :
			origin(bounds.getMin()) {
		double volume = std::max(1.0, bounds.getVolume());
		double s = std::pow(1000 / volume, 1.f / 3);

		Vec3 extents = bounds.getExtents();
		s = std::min(s, .5f / minBucketSize);

		dimx = std::max(1, (int) (extents.getX() * s));
		dimy = std::max(1, (int) (extents.getY() * s));
		dimz = std::max(1, (int) (extents.getZ() * s));

		sx = extents.getX() == 0 ? 0 : dimx / extents.getX();
		sy = extents.getY() == 0 ? 0 : dimy / extents.getY();
		sz = extents.getZ() == 0 ? 0 : dimz / extents.getZ();

		buckets.resize(dimx * dimy * dimz);
	}

	std::vector<std::vector<TYPE>> & getBuckets() {
		return buckets;
	}

	void insert(const TYPE & obj, const Vec3 & p, double radius) {
		// min cell
		int x = static_cast<int>((p.getX() - origin.getX() - radius) * sx);
		int y = static_cast<int>((p.getY() - origin.getY() - radius) * sy);
		int z = static_cast<int>((p.getZ() - origin.getZ() - radius) * sz);

		x = std::max(0, std::min(dimx - 1, x));
		y = std::max(0, std::min(dimy - 1, y));
		z = std::max(0, std::min(dimz - 1, z));

		// max cell
		int x1 = static_cast<int>((p.getX() - origin.getX() + radius) * sx);
		int y1 = static_cast<int>((p.getY() - origin.getY() + radius) * sy);
		int z1 = static_cast<int>((p.getZ() - origin.getZ() + radius) * sz);

		x1 = std::max(0, std::min(dimx - 1, x1));
		y1 = std::max(0, std::min(dimy - 1, y1));
		z1 = std::max(0, std::min(dimz - 1, z1));

		int mask = (x1 > x ? 1 : 0) + (y1 > y ? 2 : 0) + (z1 > z ? 4 : 0);

		getBucket(x, y, z).emplace_back(obj);

		if ((1 & mask) == 1) {
			getBucket(x1, y, z).emplace_back(obj);
		}
		if ((2 & mask) == 2) {
			getBucket(x, y1, z).emplace_back(obj);
		}
		if ((3 & mask) == 3) {
			getBucket(x1, y1, z).emplace_back(obj);
		}
		if ((4 & mask) == 4) {
			getBucket(x, y, z1).emplace_back(obj);
		}
		if ((5 & mask) == 5) {
			getBucket(x1, y, z1).emplace_back(obj);
		}
		if ((6 & mask) == 6) {
			getBucket(x, y1, z1).emplace_back(obj);
		}
		if ((7 & mask) == 7) {
			getBucket(x1, y1, z1).emplace_back(obj);
		}
	}

	template<typename FUNC>
	void intersection(const Vec3 & p, double radius, FUNC & fn) {
		int x = (int) ((p.getX() - origin.getX() - radius) * sx);
		int y = (int) ((p.getY() - origin.getY() - radius) * sy);
		int z = (int) ((p.getZ() - origin.getZ() - radius) * sz);
		x = std::max(0, std::min(dimx - 1, x));
		y = std::max(0, std::min(dimy - 1, y));
		z = std::max(0, std::min(dimz - 1, z));

		int x1 = (int) ((p.getX() - origin.getX() + radius) * sx);
		int y1 = (int) ((p.getY() - origin.getY() + radius) * sy);
		int z1 = (int) ((p.getZ() - origin.getZ() + radius) * sz);
		x1 = std::max(0, std::min(dimx - 1, x1));
		y1 = std::max(0, std::min(dimy - 1, y1));
		z1 = std::max(0, std::min(dimz - 1, z1));

		int mask = (x1 > x ? 1 : 0) + (y1 > y ? 2 : 0) + (z1 > z ? 4 : 0);

		for (const auto & obj : getBucket(x, y, z)) {
			fn(obj);
		}

		if ((1 & mask) == 1) {
			for (const auto & obj : getBucket(x1, y, z)) {
				fn(obj);
			}
		}
		if ((2 & mask) == 2) {
			for (const auto & obj : getBucket(x, y1, z)) {
				fn(obj);
			}
		}
		if ((3 & mask) == 3) {
			for (const auto & obj : getBucket(x1, y1, z)) {
				fn(obj);
			}
		}
		if ((4 & mask) == 4) {
			for (const auto & obj : getBucket(x, y, z1)) {
				fn(obj);
			}
		}
		if ((5 & mask) == 5) {
			for (const auto & obj : getBucket(x1, y, z1)) {
				fn(obj);
			}
		}
		if ((6 & mask) == 6) {
			for (const auto & obj : getBucket(x, y1, z1)) {
				fn(obj);
			}
		}
		if ((7 & mask) == 7) {
			for (const auto & obj : getBucket(x1, y1, z1)) {
				fn(obj);
			}
		}
	}

private:
	Vec3 origin;

	int dimx;
	int dimy;
	int dimz;

	double sx;
	double sy;
	double sz;

	std::vector<std::vector<TYPE>> buckets;

	std::vector<TYPE> & getBucket(int x, int y, int z) {
		return buckets[x + dimx * (y + dimy * z)];
	}
};

