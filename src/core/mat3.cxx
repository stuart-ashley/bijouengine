#include "mat3.h"
#include "quat.h"

#include <cassert>
#include <cmath>
#include <iostream>

/*
 *
 */
Mat3::Mat3(float r0c0, float r0c1, float r0c2, float r1c0, float r1c1,
		float r1c2, float r2c0, float r2c1, float r2c2) :
				r0c0(r0c0),
				r0c1(r0c1),
				r0c2(r0c2),
				r1c0(r1c0),
				r1c1(r1c1),
				r1c2(r1c2),
				r2c0(r2c0),
				r2c1(r2c1),
				r2c2(r2c2) {
}

/*
 *
 */
Mat3::Mat3(const Vec3 & c0, const Vec3 & c1, const Vec3 & c2) :
				r0c0(static_cast<float>(c0.getX())),
				r0c1(static_cast<float>(c1.getX())),
				r0c2(static_cast<float>(c2.getX())),
				r1c0(static_cast<float>(c0.getY())),
				r1c1(static_cast<float>(c1.getY())),
				r1c2(static_cast<float>(c2.getY())),
				r2c0(static_cast<float>(c0.getZ())),
				r2c1(static_cast<float>(c1.getZ())),
				r2c2(static_cast<float>(c2.getZ())) {
}

/*
 *
 */
Mat3::Mat3(const Mat3 & m) :
				r0c0(m.r0c0),
				r0c1(m.r0c1),
				r0c2(m.r0c2),
				r1c0(m.r1c0),
				r1c1(m.r1c1),
				r1c2(m.r1c2),
				r2c0(m.r2c0),
				r2c1(m.r2c1),
				r2c2(m.r2c2) {
}

/*
 *
 */
Mat3::Mat3(const Quat & rotation) {
	float w = rotation.getW();
	float x = rotation.getX();
	float y = rotation.getY();
	float z = rotation.getZ();

	r0c0 = 1.f - 2.f * y * y - 2.f * z * z;
	r0c1 = 2.f * x * y - 2.f * z * w;
	r0c2 = 2.f * x * z + 2.f * y * w;
	r1c0 = 2.f * x * y + 2.f * z * w;
	r1c1 = 1.f - 2.f * x * x - 2.f * z * z;
	r1c2 = 2.f * y * z - 2.f * x * w;
	r2c0 = 2.f * x * z - 2.f * y * w;
	r2c1 = 2.f * y * z + 2.f * x * w;
	r2c2 = 1.f - 2.f * x * x - 2.f * y * y;
}

/**
 * destructor
 */
Mat3::~Mat3() {
}

/**
 * get quaternion representation of matrix
 *
 * @return  matrix as quaternion
 */
Quat Mat3::asQuat() const {
	if (r0c0 + r1c1 + r2c2 > 0.f) {
		float w = .5f * std::sqrt(1.f + r0c0 + r1c1 + r2c2);
		return Quat((r2c1 - r1c2) / (4.f * w), (r0c2 - r2c0) / (4.f * w),
				(r1c0 - r0c1) / (4.f * w), w);
	}

	if (r0c0 > r1c1 && r0c0 > r2c2) {
		float x = .5f * std::sqrt(1.f + r0c0 - r1c1 - r2c2);
		return Quat(x, (r1c0 + r0c1) / (4.f * x), (r0c2 + r2c0) / (4.f * x),
				(r2c1 - r1c2) / (4.f * x));
	}

	if (r1c1 > r2c2) {
		float y = .5f * std::sqrt(1.f - r0c0 + r1c1 - r2c2);
		return Quat((r1c0 + r0c1) / (4.f * y), y, (r2c1 + r1c2) / (4.f * y),
				(r0c2 - r2c0) / (4.f * y));
	}

	float z = .5f * std::sqrt(1.f - r0c0 - r1c1 + r2c2);

	return Quat((r0c2 + r2c0) / (4.f * z), (r2c1 + r1c2) / (4.f * z), z,
			(r1c0 - r0c1) / (4.f * z));
}

/**
 * get element at given row and column
 *
 * @param row  row number
 * @param col  column number
 *
 * @return     element at row, column
 */
float Mat3::get(unsigned row, unsigned col) const {
	assert(row < 3 && col < 3);
	return m_array[row * 3 + col];
}

/**
 * calculate inverse of matrix
 *
 * @return  inverse of matrix
 */
Mat3 Mat3::inverse() const {
	float d = r0c0 * r1c1 * r2c2 + r1c0 * r2c1 * r0c2 + r2c0 * r0c1 * r1c2
			- r0c0 * r2c1 * r1c2 - r2c0 * r1c1 * r0c2 - r1c0 * r0c1 * r2c2;
	assert(std::abs(d) > .00001f);

	float m00 = r1c1 * r2c2 - r1c2 * r2c1;
	float m01 = r0c2 * r2c1 - r0c1 * r2c2;
	float m02 = r0c1 * r1c2 - r0c2 * r1c1;

	float m10 = r1c2 * r2c0 - r1c0 * r2c2;
	float m11 = r0c0 * r2c2 - r0c2 * r2c0;
	float m12 = r0c2 * r1c0 - r0c0 * r1c2;

	float m20 = r1c0 * r2c1 - r1c1 * r2c0;
	float m21 = r0c1 * r2c0 - r0c0 * r2c1;
	float m22 = r0c0 * r1c1 - r0c1 * r1c0;

	return Mat3(m00 / d, m01 / d, m02 / d, m10 / d, m11 / d, m12 / d,
			m20 / d, m21 / d, m22 / d);
}

/**
 * calculate matrix by normal
 *
 * @param n  normal to transform
 *
 * @return   transformed normal
 */
Normal Mat3::operator*(const Normal & n) const {
	float nx = n.getX();
	float ny = n.getY();
	float nz = n.getZ();
	float x = r0c0 * nx + r0c1 * ny + r0c2 * nz;
	float y = r1c0 * nx + r1c1 * ny + r1c2 * nz;
	float z = r2c0 * nx + r2c1 * ny + r2c2 * nz;
	return Normal(x, y, z);
}

/**
 * calculate matrix by matrix
 *
 * @param other  matrix to transform
 *
 * @return       transformed matrix
 */
Mat3 Mat3::operator*(const Mat3 & other) const {
	float m00 = r0c0 * other.r0c0 + r0c1 * other.r1c0 + r0c2 * other.r2c0;
	float m01 = r0c0 * other.r0c1 + r0c1 * other.r1c1 + r0c2 * other.r2c1;
	float m02 = r0c0 * other.r0c2 + r0c1 * other.r1c2 + r0c2 * other.r2c2;

	float m10 = r1c0 * other.r0c0 + r1c1 * other.r1c0 + r1c2 * other.r2c0;
	float m11 = r1c0 * other.r0c1 + r1c1 * other.r1c1 + r1c2 * other.r2c1;
	float m12 = r1c0 * other.r0c2 + r1c1 * other.r1c2 + r1c2 * other.r2c2;

	float m20 = r2c0 * other.r0c0 + r2c1 * other.r1c0 + r2c2 * other.r2c0;
	float m21 = r2c0 * other.r0c1 + r2c1 * other.r1c1 + r2c2 * other.r2c1;
	float m22 = r2c0 * other.r0c2 + r2c1 * other.r1c2 + r2c2 * other.r2c2;

	return Mat3(m00, m01, m02, m10, m11, m12, m20, m21, m22);
}

/**
 * calculate matrix by vec3
 *
 * @param v  vector to transform
 *
 * @return   transformed vector
 */
Vec3 Mat3::operator*(const Vec3 & v) const {
	return Vec3(r0c0 * v.getX() + r0c1 * v.getY() + r0c2 * v.getZ(),
			r1c0 * v.getX() + r1c1 * v.getY() + r1c2 * v.getZ(),
			r2c0 * v.getX() + r2c1 * v.getY() + r2c2 * v.getZ());
}

/**
 * subtract other matrix from this matrix
 *
 * @param other  matrix to subtract
 */
Mat3 & Mat3::operator-=(const Mat3 & other) {
	for (size_t i = 0, n = 9; i < n; ++i) {
		m_array[i] -= other.m_array[i];
	}
	return *this;
}

/**
 * calculate this matrix plus other matrix
 *
 * @param other  matrix to add
 *
 * @return       this matrix plus other matrix
 */
Mat3 Mat3::operator+(const Mat3 & other) const {
	return Mat3(r0c0 + other.r0c0, r0c1 + other.r0c1, r0c2 + other.r0c2,
			r1c0 + other.r1c0, r1c1 + other.r1c1, r1c2 + other.r1c2,
			r2c0 + other.r2c0, r2c1 + other.r2c1, r2c2 + other.r2c2);
}

/*
 *
 */
std::string Mat3::toString() const {
	return "Mat3( " + std::to_string(r0c0) + "," + std::to_string(r0c1) + ", "
			+ std::to_string(r0c2) + " )" + "    ( " + std::to_string(r1c0)
			+ "," + std::to_string(r1c1) + ", " + std::to_string(r1c2) + " )"
			+ "    ( " + std::to_string(r2c0) + "," + std::to_string(r2c1)
			+ ", " + std::to_string(r2c2) + " )";
}
