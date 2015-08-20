#include "mat4.h"
#include "quat.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"

#include <cmath>

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {
		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 16);

			// row, column
			float m[4][4];

			for (int r = 0; r < 4; ++r) {
				for (int c = 0; c < 4; ++c) {
					m[r][c] = static_cast<float>(getNumericArg(stack, r * 4 + c));
				}
			}
			stack.emplace(
					std::make_shared<Mat4>(m[0][0], m[0][1], m[0][2], m[0][3],
							m[1][0], m[1][1], m[1][2], m[1][3],
							m[2][0], m[2][1], m[2][2], m[2][3],
							m[3][0], m[3][1], m[3][2], m[3][3]));
		}
	};
}

/**
 * constructor, fill all elements with zero
 */
Mat4::Mat4() :
				m_r0c0(0), m_r0c1(0), m_r0c2(0), m_r0c3(0),
				m_r1c0(0), m_r1c1(0), m_r1c2(0), m_r1c3(0),
				m_r2c0(0), m_r2c1(0), m_r2c2(0), m_r2c3(0),
				m_r3c0(0), m_r3c1(0), m_r3c2(0), m_r3c3(0) {
}

/**
 * construct from rotation & translation
 *
 * @param r	rotation
 * @param t	translation
 */
Mat4::Mat4(const Quat & r, const Vec3 & t) {
	m_r0c0 = 1 - 2 * (r.getY() * r.getY() + r.getZ() * r.getZ());
	m_r1c0 = 2 * (r.getX() * r.getY() + r.getW() * r.getZ());
	m_r2c0 = 2 * (r.getX() * r.getZ() - r.getW() * r.getY());
	m_r3c0 = 0;

	m_r0c1 = 2 * (r.getX() * r.getY() - r.getW() * r.getZ());
	m_r1c1 = 1 - 2 * (r.getX() * r.getX() + r.getZ() * r.getZ());
	m_r2c1 = 2 * (r.getY() * r.getZ() + r.getW() * r.getX());
	m_r3c1 = 0;

	m_r0c2 = 2 * (r.getX() * r.getZ() + r.getW() * r.getY());
	m_r1c2 = 2 * (r.getY() * r.getZ() - r.getW() * r.getX());
	m_r2c2 = 1 - 2 * (r.getX() * r.getX() + r.getY() * r.getY());
	m_r3c2 = 0;

	m_r0c3 = static_cast<float>(t.getX());
	m_r1c3 = static_cast<float>(t.getY());
	m_r2c3 = static_cast<float>(t.getZ());
	m_r3c3 = 1;
}

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
Mat4::Mat4(float r0c0, float r0c1, float r0c2, float r0c3, float r1c0,
		float r1c1, float r1c2, float r1c3, float r2c0, float r2c1, float r2c2,
		float r2c3, float r3c0, float r3c1, float r3c2, float r3c3) :
				m_r0c0(r0c0), m_r0c1(r0c1), m_r0c2(r0c2), m_r0c3(r0c3),
				m_r1c0(r1c0), m_r1c1(r1c1), m_r1c2(r1c2), m_r1c3(r1c3),
				m_r2c0(r2c0), m_r2c1(r2c1), m_r2c2(r2c2), m_r2c3(r2c3),
				m_r3c0(r3c0), m_r3c1(r3c1), m_r3c2(r3c2), m_r3c3(r3c3) {
}

/**
 * calculate determinant
 *
 * @result matrix determinant
 */
float Mat4::determinant() const {
	return m_r0c3 * m_r1c2 * m_r2c1 * m_r3c0
			- m_r0c2 * m_r1c3 * m_r2c1 * m_r3c0
			- m_r0c3 * m_r1c1 * m_r2c2 * m_r3c0
			+ m_r0c1 * m_r1c3 * m_r2c2 * m_r3c0
			+ m_r0c2 * m_r1c1 * m_r2c3 * m_r3c0
			- m_r0c1 * m_r1c2 * m_r2c3 * m_r3c0
			- m_r0c3 * m_r1c2 * m_r2c0 * m_r3c1
			+ m_r0c2 * m_r1c3 * m_r2c0 * m_r3c1
			+ m_r0c3 * m_r1c0 * m_r2c2 * m_r3c1
			- m_r0c0 * m_r1c3 * m_r2c2 * m_r3c1
			- m_r0c2 * m_r1c0 * m_r2c3 * m_r3c1
			+ m_r0c0 * m_r1c2 * m_r2c3 * m_r3c1
			+ m_r0c3 * m_r1c1 * m_r2c0 * m_r3c2
			- m_r0c1 * m_r1c3 * m_r2c0 * m_r3c2
			- m_r0c3 * m_r1c0 * m_r2c1 * m_r3c2
			+ m_r0c0 * m_r1c3 * m_r2c1 * m_r3c2
			+ m_r0c1 * m_r1c0 * m_r2c3 * m_r3c2
			- m_r0c0 * m_r1c1 * m_r2c3 * m_r3c2
			- m_r0c2 * m_r1c1 * m_r2c0 * m_r3c3
			+ m_r0c1 * m_r1c2 * m_r2c0 * m_r3c3
			+ m_r0c2 * m_r1c0 * m_r2c1 * m_r3c3
			- m_r0c0 * m_r1c2 * m_r2c1 * m_r3c3
			- m_r0c1 * m_r1c0 * m_r2c2 * m_r3c3
			+ m_r0c0 * m_r1c1 * m_r2c2 * m_r3c3;
}

/**
 * get 3x3 matrix comprising first 3 rows, first 3 columns
 *
 * @return 3x3 rotation scale matrix
 */
Mat3 Mat4::getRotationScale() const {
	return Mat3(m_r0c0, m_r0c1, m_r0c2, m_r1c0, m_r1c1, m_r1c2, m_r2c0, m_r2c1,
			m_r2c2);
}

/**
 * get first 3 rows of final column divided by element at final row final column
 *
 * @return translation
 */
Vec3 Mat4::getTranslation() const {
	return Vec3(m_r3c0 / m_r3c3, m_r3c1 / m_r3c3, m_r3c2 / m_r3c3);
}

/**
 * calculate inverse of matrix
 *
 * @return  matrix inverse
 */
Mat4 Mat4::inverse() const {
	float r0c0 = m_r1c1 * m_r2c2 * m_r3c3 + m_r1c2 * m_r2c3 * m_r3c1
			+ m_r1c3 * m_r2c1 * m_r3c2 - m_r1c1 * m_r2c3 * m_r3c2
			- m_r1c2 * m_r2c1 * m_r3c3 - m_r1c3 * m_r2c2 * m_r3c1;
	float r0c1 = m_r0c1 * m_r2c3 * m_r3c2 + m_r0c2 * m_r2c1 * m_r3c3
			+ m_r0c3 * m_r2c2 * m_r3c1 - m_r0c1 * m_r2c2 * m_r3c3
			- m_r0c2 * m_r2c3 * m_r3c1 - m_r0c3 * m_r2c1 * m_r3c2;
	float r0c2 = m_r0c1 * m_r1c2 * m_r3c3 + m_r0c2 * m_r1c3 * m_r3c1
			+ m_r0c3 * m_r1c1 * m_r3c2 - m_r0c1 * m_r1c3 * m_r3c2
			- m_r0c2 * m_r1c1 * m_r3c3 - m_r0c3 * m_r1c2 * m_r3c1;
	float r0c3 = m_r0c1 * m_r1c3 * m_r2c2 + m_r0c2 * m_r1c1 * m_r2c3
			+ m_r0c3 * m_r1c2 * m_r2c1 - m_r0c1 * m_r1c2 * m_r2c3
			- m_r0c2 * m_r1c3 * m_r2c1 - m_r0c3 * m_r1c1 * m_r2c2;
	float r1c0 = m_r1c0 * m_r2c3 * m_r3c2 + m_r1c2 * m_r2c0 * m_r3c3
			+ m_r1c3 * m_r2c2 * m_r3c0 - m_r1c0 * m_r2c2 * m_r3c3
			- m_r1c2 * m_r2c3 * m_r3c0 - m_r1c3 * m_r2c0 * m_r3c2;
	float r1c1 = m_r0c0 * m_r2c2 * m_r3c3 + m_r0c2 * m_r2c3 * m_r3c0
			+ m_r0c3 * m_r2c0 * m_r3c2 - m_r0c0 * m_r2c3 * m_r3c2
			- m_r0c2 * m_r2c0 * m_r3c3 - m_r0c3 * m_r2c2 * m_r3c0;
	float r1c2 = m_r0c0 * m_r1c3 * m_r3c2 + m_r0c2 * m_r1c0 * m_r3c3
			+ m_r0c3 * m_r1c2 * m_r3c0 - m_r0c0 * m_r1c2 * m_r3c3
			- m_r0c2 * m_r1c3 * m_r3c0 - m_r0c3 * m_r1c0 * m_r3c2;
	float r1c3 = m_r0c0 * m_r1c2 * m_r2c3 + m_r0c2 * m_r1c3 * m_r2c0
			+ m_r0c3 * m_r1c0 * m_r2c2 - m_r0c0 * m_r1c3 * m_r2c2
			- m_r0c2 * m_r1c0 * m_r2c3 - m_r0c3 * m_r1c2 * m_r2c0;
	float r2c0 = m_r1c0 * m_r2c1 * m_r3c3 + m_r1c1 * m_r2c3 * m_r3c0
			+ m_r1c3 * m_r2c0 * m_r3c1 - m_r1c0 * m_r2c3 * m_r3c1
			- m_r1c1 * m_r2c0 * m_r3c3 - m_r1c3 * m_r2c1 * m_r3c0;
	float r2c1 = m_r0c0 * m_r2c3 * m_r3c1 + m_r0c1 * m_r2c0 * m_r3c3
			+ m_r0c3 * m_r2c1 * m_r3c0 - m_r0c0 * m_r2c1 * m_r3c3
			- m_r0c1 * m_r2c3 * m_r3c0 - m_r0c3 * m_r2c0 * m_r3c1;
	float r2c2 = m_r0c0 * m_r1c1 * m_r3c3 + m_r0c1 * m_r1c3 * m_r3c0
			+ m_r0c3 * m_r1c0 * m_r3c1 - m_r0c0 * m_r1c3 * m_r3c1
			- m_r0c1 * m_r1c0 * m_r3c3 - m_r0c3 * m_r1c1 * m_r3c0;
	float r2c3 = m_r0c0 * m_r1c3 * m_r2c1 + m_r0c1 * m_r1c0 * m_r2c3
			+ m_r0c3 * m_r1c1 * m_r2c0 - m_r0c0 * m_r1c1 * m_r2c3
			- m_r0c1 * m_r1c3 * m_r2c0 - m_r0c3 * m_r1c0 * m_r2c1;
	float r3c0 = m_r1c0 * m_r2c2 * m_r3c1 + m_r1c1 * m_r2c0 * m_r3c2
			+ m_r1c2 * m_r2c1 * m_r3c0 - m_r1c0 * m_r2c1 * m_r3c2
			- m_r1c1 * m_r2c2 * m_r3c0 - m_r1c2 * m_r2c0 * m_r3c1;
	float r3c1 = m_r0c0 * m_r2c1 * m_r3c2 + m_r0c1 * m_r2c2 * m_r3c0
			+ m_r0c2 * m_r2c0 * m_r3c1 - m_r0c0 * m_r2c2 * m_r3c1
			- m_r0c1 * m_r2c0 * m_r3c2 - m_r0c2 * m_r2c1 * m_r3c0;
	float r3c2 = m_r0c0 * m_r1c2 * m_r3c1 + m_r0c1 * m_r1c0 * m_r3c2
			+ m_r0c2 * m_r1c1 * m_r3c0 - m_r0c0 * m_r1c1 * m_r3c2
			- m_r0c1 * m_r1c2 * m_r3c0 - m_r0c2 * m_r1c0 * m_r3c1;
	float r3c3 = m_r0c0 * m_r1c1 * m_r2c2 + m_r0c1 * m_r1c2 * m_r2c0
			+ m_r0c2 * m_r1c0 * m_r2c1 - m_r0c0 * m_r1c2 * m_r2c1
			- m_r0c1 * m_r1c0 * m_r2c2 - m_r0c2 * m_r1c1 * m_r2c0;

	float d = determinant();
	assert(std::abs(d) > 0.00001);

	return Mat4(r0c0 / d, r0c1 / d, r0c2 / d, r0c3 / d, r1c0 / d, r1c1 / d,
			r1c2 / d, r1c3 / d, r2c0 / d, r2c1 / d, r2c2 / d, r2c3 / d,
			r3c0 / d, r3c1 / d, r3c2 / d, r3c3 / d);
}

/**
 * make this matrix equal to matrix product of m and n
 *
 * M = m * n
 *
 * @param m left matrix
 * @param n right matrix
 */
void Mat4::mul(const Mat4 & m, const Mat4 & n) {
	float r0c0 = m.m_r0c0 * n.m_r0c0 + m.m_r0c1 * n.m_r1c0 + m.m_r0c2 * n.m_r2c0
			+ m.m_r0c3 * n.m_r3c0;
	float r0c1 = m.m_r0c0 * n.m_r0c1 + m.m_r0c1 * n.m_r1c1 + m.m_r0c2 * n.m_r2c1
			+ m.m_r0c3 * n.m_r3c1;
	float r0c2 = m.m_r0c0 * n.m_r0c2 + m.m_r0c1 * n.m_r1c2 + m.m_r0c2 * n.m_r2c2
			+ m.m_r0c3 * n.m_r3c2;
	float r0c3 = m.m_r0c0 * n.m_r0c3 + m.m_r0c1 * n.m_r1c3 + m.m_r0c2 * n.m_r2c3
			+ m.m_r0c3 * n.m_r3c3;

	float r1c0 = m.m_r1c0 * n.m_r0c0 + m.m_r1c1 * n.m_r1c0 + m.m_r1c2 * n.m_r2c0
			+ m.m_r1c3 * n.m_r3c0;
	float r1c1 = m.m_r1c0 * n.m_r0c1 + m.m_r1c1 * n.m_r1c1 + m.m_r1c2 * n.m_r2c1
			+ m.m_r1c3 * n.m_r3c1;
	float r1c2 = m.m_r1c0 * n.m_r0c2 + m.m_r1c1 * n.m_r1c2 + m.m_r1c2 * n.m_r2c2
			+ m.m_r1c3 * n.m_r3c2;
	float r1c3 = m.m_r1c0 * n.m_r0c3 + m.m_r1c1 * n.m_r1c3 + m.m_r1c2 * n.m_r2c3
			+ m.m_r1c3 * n.m_r3c3;

	float r2c0 = m.m_r2c0 * n.m_r0c0 + m.m_r2c1 * n.m_r1c0 + m.m_r2c2 * n.m_r2c0
			+ m.m_r2c3 * n.m_r3c0;
	float r2c1 = m.m_r2c0 * n.m_r0c1 + m.m_r2c1 * n.m_r1c1 + m.m_r2c2 * n.m_r2c1
			+ m.m_r2c3 * n.m_r3c1;
	float r2c2 = m.m_r2c0 * n.m_r0c2 + m.m_r2c1 * n.m_r1c2 + m.m_r2c2 * n.m_r2c2
			+ m.m_r2c3 * n.m_r3c2;
	float r2c3 = m.m_r2c0 * n.m_r0c3 + m.m_r2c1 * n.m_r1c3 + m.m_r2c2 * n.m_r2c3
			+ m.m_r2c3 * n.m_r3c3;

	float r3c0 = m.m_r3c0 * n.m_r0c0 + m.m_r3c1 * n.m_r1c0 + m.m_r3c2 * n.m_r2c0
			+ m.m_r3c3 * n.m_r3c0;
	float r3c1 = m.m_r3c0 * n.m_r0c1 + m.m_r3c1 * n.m_r1c1 + m.m_r3c2 * n.m_r2c1
			+ m.m_r3c3 * n.m_r3c1;
	float r3c2 = m.m_r3c0 * n.m_r0c2 + m.m_r3c1 * n.m_r1c2 + m.m_r3c2 * n.m_r2c2
			+ m.m_r3c3 * n.m_r3c2;
	float r3c3 = m.m_r3c0 * n.m_r0c3 + m.m_r3c1 * n.m_r1c3 + m.m_r3c2 * n.m_r2c3
			+ m.m_r3c3 * n.m_r3c3;

	m_r0c0 = r0c0;
	m_r0c1 = r0c1;
	m_r0c2 = r0c2;
	m_r0c3 = r0c3;

	m_r1c0 = r1c0;
	m_r1c1 = r1c1;
	m_r1c2 = r1c2;
	m_r1c3 = r1c3;

	m_r2c0 = r2c0;
	m_r2c1 = r2c1;
	m_r2c2 = r2c2;
	m_r2c3 = r2c3;

	m_r3c0 = r3c0;
	m_r3c1 = r3c1;
	m_r3c2 = r3c2;
	m_r3c3 = r3c3;
}

/**
 * make this matrix equal to matrix product of self and m
 *
 * M = M * m
 *
 * @param m right matrix
 */
void Mat4::mul(const Mat4 & m) {
	mul(*this, m);
}

/**
 * matrix as string
 *
 * @return string version of matrix
 */
OVERRIDE std::string Mat4::toString() const {
	return "Mat4([" + std::to_string(m_r0c0) + ", "
			+ std::to_string(m_r0c1) + ", "
			+ std::to_string(m_r0c2) + ", "
			+ std::to_string(m_r0c3) + "]\n" +
			"     [" + std::to_string(m_r1c0) + ", "
			+ std::to_string(m_r1c1) + ", "
			+ std::to_string(m_r1c2) + ", "
			+ std::to_string(m_r1c3) + "]\n"
			+ "     [" + std::to_string(m_r2c0)	+ ", "
			+ std::to_string(m_r2c1) + ", "
			+ std::to_string(m_r2c2) + ", "
			+ std::to_string(m_r2c3) + "]\n"
			+ "     [" + std::to_string(m_r3c0) + ", "
			+ std::to_string(m_r3c1) + ", "
			+ std::to_string(m_r3c2) + ", "
			+ std::to_string(m_r3c3) + "])";
}

/**
 * transform vector by this matrix
 *
 * v = M * v
 *
 * @param v vector to transform
 */
void Mat4::transform(Vec3 & v) const {
	auto x = m_r0c0 * v.getX() + m_r0c1 * v.getY() + m_r0c2 * v.getZ()
			+ m_r0c3;
	auto y = m_r1c0 * v.getX() + m_r1c1 * v.getY() + m_r1c2 * v.getZ()
			+ m_r1c3;
	auto z = m_r2c0 * v.getX() + m_r2c1 * v.getY() + m_r2c2 * v.getZ()
			+ m_r2c3;
	auto w = m_r3c0 * v.getX() + m_r3c1 * v.getY() + m_r3c2 * v.getZ()
			+ m_r3c3;

	double s = 1.0 / w;
	v.set(x * s, y * s, z * s);
}

/**
 * get script object factory for Mat4
 *
 * @return  Mat4 factory
 */
STATIC const ScriptObjectPtr & Mat4::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}

/**
 * get identity matrix
 *
 * @return  identity matrix
 */
STATIC const Mat4 & Mat4::identity() {
	static Mat4 m(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	return m;
}
