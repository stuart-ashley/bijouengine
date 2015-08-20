#pragma once

#include "normal.h"
#include "vec3.h"

#include "../scripting/scriptObject.h"

#include <memory>

class Quat;

class Mat3: public ScriptObject {
public:

	Mat3(float r0c0, float r0c1, float r0c2, float r1c0, float r1c1, float r1c2,
			float r2c0, float r2c1, float r2c2);

	Mat3(const Vec3 & c0, const Vec3 & c1, const Vec3 & c2);

	Mat3(const Mat3 & m);

	Mat3(const Quat & rotation);

	/**
	 * destructor
	 */
	~Mat3();

	/**
	 * get quaternion representation of matrix
	 *
	 * @return  matrix as quaternion
	 */
	Quat asQuat() const;

	/**
	 * get element at given row and column
	 *
	 * @param row  row number
	 * @param col  column number
	 *
	 * @return     element at row, column
	 */
	float get(unsigned row, unsigned col) const;

	/**
	 * calculate inverse of matrix
	 *
	 * @return  inverse of matrix
	 */
	Mat3 inverse() const;

	/**
	 * calculate matrix by matrix
	 *
	 * @param other  matrix to transform
	 *
	 * @return       transformed matrix
	 */
	Mat3 operator*(const Mat3 & other) const;

	/**
	 * calculate matrix by normal
	 *
	 * @param n  normal to transform
	 *
	 * @return   transformed normal
	 */
	Normal operator*(const Normal & n) const;

	/**
	 * calculate matrix by vec3
	 *
	 * @param v  vector to transform
	 *
	 * @return   transformed vector
	 */
	Vec3 operator*(const Vec3 & v) const;

	/**
	 * calculate this matrix plus other matrix
	 *
	 * @param other  matrix to add
	 *
	 * @return       this matrix plus other matrix
	 */
	Mat3 operator+(const Mat3 & other) const;

	/**
	 * subtract other matrix from this matrix
	 *
	 * @param other  matrix to subtract
	 */
	Mat3 & operator-=(const Mat3 & other);

	std::string toString() const;

private:
	union {
		struct {
			float r0c0, r0c1, r0c2, r1c0, r1c1, r1c2, r2c0, r2c1, r2c2;
		};
		float m_array[9];
	};
};
