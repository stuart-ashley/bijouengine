#pragma once

#include "mat3.h"
#include "vec3.h"

#include "../scripting/scriptObject.h"

#include <cassert>

class Mat4 final: public ScriptObject {
public:
	/**
	 * constructor, fill all elements with zero
	 */
	Mat4();

	/**
	 * copy constructor
	 */
	Mat4(const Mat4 &) = default;

	/**
	 * construct from rotation & translation
	 *
	 * @param r	rotation
	 * @param t	translation
	 */
	Mat4(const Quat & r, const Vec3 & t);

	/**
	 * construct from elements
	 *
	 * @param r0c0 row 0 column 0
	 * @param r0c1 row 0 column 1
	 * @param r0c2 row 0 column 2
	 * @param r0c3 row 0 column 3
	 * @param r1c0 row 1 column 0
	 * @param r1c1 row 1 column 1
	 * @param r1c2 row 1 column 2
	 * @param r1c3 row 1 column 3
	 * @param r2c0 row 2 column 0
	 * @param r2c1 row 2 column 1
	 * @param r2c2 row 2 column 2
	 * @param r2c3 row 2 column 3
	 * @param r3c0 row 3 column 0
	 * @param r3c1 row 3 column 1
	 * @param r3c2 row 3 column 2
	 * @param r3c3 row 3 column 3
	 */
	Mat4(float r0c0, float r0c1, float r0c2, float r0c3, float r1c0, float r1c1,
			float r1c2, float r1c3, float r2c0, float r2c1, float r2c2,
			float r2c3, float r3c0, float r3c1, float r3c2, float r3c3);

	/**
	 * default destructor
	 */
	inline virtual ~Mat4() = default;

	/**
	 * calculate determinant
	 *
	 * @result matrix determinant
	 */
	float determinant() const;

	/**
	 * get an element
	 *
	 * @param row     row 0..3
	 * @param column  column 0..3
	 *
	 * @return        element at given row, column
	 */
	inline float get(unsigned row, unsigned column) const {
		assert(row < 4 && column < 4);
		return m_array[row * 4 + column];
	}

	/**
	 * get 3x3 matrix comprising first 3 rows, first 3 columns
	 *
	 * @return 3x3 rotation scale matrix
	 */
	Mat3 getRotationScale() const;

	/**
	 * get first 3 rows of final column divided by element at final row final column
	 *
	 * @return translation
	 */
	Vec3 getTranslation() const;

	/**
	 * calculate inverse of matrix
	 *
	 * @return  matrix inverse
	 */
	Mat4 inverse() const;

	/**
	 * make this matrix equal to matrix product of m and n
	 *
	 * M = m * n
	 *
	 * @param m left matrix
	 * @param n right matrix
	 */
	void mul(const Mat4 & m, const Mat4 & n);

	/**
	 * make this matrix equal to matrix product of self and m
	 *
	 * M = M * m
	 *
	 * @param m right matrix
	 */
	void mul(const Mat4 & m);

	/**
	 * matrix as string
	 *
	 * @return string version of matrix
	 */
	std::string toString() const override;

	/**
	 * transform vector by this matrix
	 *
	 * v = M * v
	 *
	 * @param v vector to transform
	 */
	void transform(Vec3 & v) const;

	/**
	 * get script object factory for Mat4
	 *
	 * @return  Mat4 factory
	 */
	static const ScriptObjectPtr & getFactory();

	/**
	 * get identity matrix
	 *
	 * @return  identity matrix
	 */
	static const Mat4 & identity();

private:
	union {
		struct {
			float m_r0c0, m_r0c1, m_r0c2, m_r0c3, m_r1c0, m_r1c1, m_r1c2,
					m_r1c3, m_r2c0, m_r2c1, m_r2c2, m_r2c3, m_r3c0, m_r3c1,
					m_r3c2, m_r3c3;
		};
		float m_array[16];
	};
};
