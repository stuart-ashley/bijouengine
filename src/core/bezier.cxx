#define _USE_MATH_DEFINES
#include "bezier.h"

#include "binary.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

struct Bezier::impl {
	std::string name;
	Binary binary;
	/** array of ( bwdx, bwdy, px, py, fwdx, fwdy ) */
	std::vector<float> ctrlPoints;
	bool valid;

	impl(const Binary & binary) :
			name(binary.getName()), binary(binary), valid(false) {
	}

	float calcY(float x, float p0x, float p0y, float p1x, float p1y, float p2x,
			float p2y, float p3x, float p3y) const {
		double t;
		double a = p3x - 3 * p2x + 3 * p1x - p0x;
		double b = 3 * p2x - 6 * p1x + 3 * p0x;
		double c = 3 * p1x - 3 * p0x;
		double d = p0x - x;

		double p = -b * b / (3 * a * a) + c / a;
		double thirdPCubed = (p / 3) * (p / 3) * (p / 3);
		double q = 2 * b * b * b / (27 * a * a * a) - c * b / (3 * a * a)
				+ d / a;
		double dis = (q / 2) * (q / 2) + thirdPCubed;

		if (dis < 0) {
			assert(p < 0);

			// using trigonometric method, find three real roots
			double r = std::sqrt(-thirdPCubed);
			double theta = 1. / 3 * std::acos(-q / 2 / r);
			double k0 = 2 * std::pow(r, 1. / 3);
			double k1 = 2 * M_PI / 3;
			double k2 = b / (3 * a);

			double t1 = k0 * std::cos(theta) - k2;
			double t2 = k0 * std::cos(theta + k1) - k2;
			double t3 = k0 * std::cos(theta + 2 * k1) - k2;

			if (t1 >= 0 && t1 <= 1) {
				assert((t2 < 0 || t2 > 1) && (t3 < 0 || t3 > 1));
				t = t1;
			} else if (t2 >= 0 && t2 <= 1) {
				assert(t3 < 0 || t3 > 1);
				t = t2;
			} else {
				assert(t3 >= 0 && t3 <= 1);
				t = t3;
			}
		} else {
			// using Cardano's method, find one real root
			double y1 = -q / 2 + std::sqrt(dis);
			double y2 = -q / 2 - std::sqrt(dis);
			double x1 = std::copysign(std::pow(std::abs(y1), 1. / 3), y1);
			double x2 = std::copysign(std::pow(std::abs(y2), 1. / 3), y2);
			t = (x1 + x2) - (b / (3 * a));
		}

		double y = (1 - t) * (1 - t) * (1 - t) * p0y;
		y += 3 * (1 - t) * (1 - t) * t * p1y;
		y += 3 * (1 - t) * t * t * p2y;
		y += t * t * t * p3y;
		return (float) y;
	}
};

/**
 * constructor
 *
 * @param binary
 */
Bezier::Bezier(const Binary & binary) :
		pimpl(new impl(binary)) {
}

/**
 * get name of bezier curve
 *
 * @return  name of curve
 */
const std::string & Bezier::getName() const {
	return pimpl->name;
}

/**
 * get y for given x
 *
 * @param x  x value to find y for
 *
 * @return   y value for given x
 */
float Bezier::getY(float x) const {
	float tolerance = .0001f;

	const auto & firstX = pimpl->ctrlPoints[2];
	const auto & lastX = pimpl->ctrlPoints[pimpl->ctrlPoints.size() - 4];

	if (x < firstX + tolerance) {
		return pimpl->ctrlPoints[3];
	}
	if (x > lastX - tolerance) {
		return pimpl->ctrlPoints[pimpl->ctrlPoints.size() - 3];
	}

	size_t sidx = 2;
	size_t eidx = pimpl->ctrlPoints.size() - 4;
	float sx = firstX;
	float ex = lastX;
	while (true) {
		float t = (x - sx) / (ex - sx);
		size_t idx = std::min(
				static_cast<size_t>(t * static_cast<float>(eidx - sidx) / 6) * 6
						+ sidx, eidx - 6);

		// p0 - previous point
		// p1 - previous forward control
		// p2 - current backward control
		// p3 - current point
		float p0x = pimpl->ctrlPoints[idx];
		float p0y = pimpl->ctrlPoints[idx + 1];
		float p1x = pimpl->ctrlPoints[idx + 2];
		float p1y = pimpl->ctrlPoints[idx + 3];
		float p2x = pimpl->ctrlPoints[idx + 4];
		float p2y = pimpl->ctrlPoints[idx + 5];
		float p3x = pimpl->ctrlPoints[idx + 6];
		float p3y = pimpl->ctrlPoints[idx + 7];

		if (x < p0x - tolerance) {
			eidx = idx;
			ex = p0x;
			continue;
		}
		if (x > p3x + tolerance) {
			sidx = idx + 6;
			ex = p3x;
			continue;
		}
		return pimpl->calcY(x, p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
	}
	return 0;
}

/**
 * validate curve (ie check loaded)
 *
 * @return  true if valid, false otherwise
 */
bool Bezier::validate() const {
	if (pimpl->valid) {
		return true;
	}
	if (pimpl->binary.valid()) {
		// check divisible by 6 floats
		assert(pimpl->binary.getByteCount() % 24 == 0);

		size_t count = pimpl->binary.getByteCount() / sizeof(float);

		pimpl->ctrlPoints.reserve(count);

		auto floats = reinterpret_cast<const float *>(pimpl->binary.getData());

		pimpl->ctrlPoints.insert(pimpl->ctrlPoints.end(), floats,
				floats + count);

		assert(pimpl->ctrlPoints.size() == count);

		pimpl->valid = true;
		return true;
	}
	return false;
}
