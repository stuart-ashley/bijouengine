#include "transform.h"

#include "../scripting/bool.h"
#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"

#include <cassert>
#include <cmath>
#include <unordered_map>

namespace {

	/*
	 *
	 */
	class Factory: public Executable {
	public:

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 2);

			auto t = getArg<Vec3>("Vec3", stack, 1);
			auto r = getArg<Quat>("Quat", stack, 2);

			stack.emplace(std::make_shared<Transform>(t, r));
		}
	};

	/*
	 *
	 */
	class Mul: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto e = stack.top();
			stack.pop();

			if (typeid(*e) == typeid(Transform)) {
				Transform result(*std::static_pointer_cast<Transform>(self));
				result.transform(*std::static_pointer_cast<Transform>(e));
				stack.emplace(std::make_shared<Transform>(result));
			} else if (typeid(*e) == typeid(Vec3)) {
				Vec3 point(*std::static_pointer_cast<Vec3>(e));
				std::static_pointer_cast<Transform>(self)->transformPoint(
						point);
				stack.emplace(std::make_shared<Vec3>(point));
			} else {
				scriptExecutionAssert(false, "Require Transform or Vec3 ");
			}
		}
	};
}

/**
 * transform as mat4
 *
 * @return mat4 of transform
 */
Mat4 Transform::asMat4() const {
	// row major
	return Mat4(1 - 2 * ry * ry - 2 * rz * rz,
			2 * rx * ry - 2 * rz * rw,
			2 * rx * rz + 2 * ry * rw,
			static_cast<float>(tx),
			2 * rx * ry + 2 * rz * rw,
			1 - 2 * rx * rx - 2 * rz * rz,
			2 * ry * rz - 2 * rx * rw,
			static_cast<float>(ty),
			2 * rx * rz - 2 * ry * rw,
			2 * ry * rz + 2 * rx * rw,
			1 - 2 * rx * rx - 2 * ry * ry,
			static_cast<float>(tz),
			0, 0, 0, 1);
}

/**
 * calculate distance from this transform to other transform
 *
 * @param other transform to calculate distance to
 *
 * @return distance from this to other
 */
double Transform::distanceTo(const Transform & other) const {
	double x = tx - other.tx;
	double y = ty - other.ty;
	double z = tz - other.tz;

	return std::sqrt(x * x + y * y + z * z);
}

/**
 * get inverse of transform rotation in matrix form
 *
 * @return inverse of transform rotation
 */
Mat3 Transform::getInverseRotationMatrix() const {
	// row major
	return Mat3(1 - 2 * ry * ry - 2 * rz * rz,
			2 * rx * ry + 2 * rz * rw,
			2 * rx * rz - 2 * ry * rw,
			2 * rx * ry - 2 * rz * rw,
			1 - 2 * rx * rx - 2 * rz * rz,
			2 * ry * rz + 2 * rx * rw,
			2 * rx * rz + 2 * ry * rw,
			2 * ry * rz - 2 * rx * rw,
			1 - 2 * rx * rx - 2 * ry * ry);
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
std::shared_ptr<ScriptObject> Transform::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__mul__", std::make_shared<Mul>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * get transform rotation as matrix
 *
 * @return transform rotation
 */
Mat3 Transform::getRotationMatrix() const {
	// row major
	return Mat3(1 - 2 * (ry * ry + rz * rz),
			2 * (rx * ry - rz * rw),
			2 * (rx * rz + ry * rw),
			2 * (rx * ry + rz * rw),
			1 - 2 * (rx * rx + rz * rz),
			2 * (ry * rz - rx * rw),
			2 * (rx * rz - ry * rw),
			2 * (ry * rz + rx * rw),
			1 - 2 * (rx * rx + ry * ry));
}

/**
 * interpolate between 'this' and 'other', replacing 'this'
 *
 * t = 0, 'this' unchanged
 * t = 1, 'this' = other
 *
 * T = (1-t) * T + t * T'
 * R = (1-t) * R + t * R'
 *
 * @param other
 * @param t
 */
void Transform::interpolate(const Transform & other, float t) {
	Quat a(rx, ry, rz, rw);
	Quat b(other.rx, other.ry, other.rz, other.rw);
	a.interpolate(b, t);
	setRotation(a);
	tx = (1 - t) * tx + t * other.tx;
	ty = (1 - t) * ty + t * other.ty;
	tz = (1 - t) * tz + t * other.tz;

	assert(std::isfinite(tx));
	assert(std::isfinite(ty));
	assert(std::isfinite(tz));
}

/**
 * calculate inverse of transform
 *
 * @return inverse of transform
 */
Transform Transform::inverse() const {
	double x = ry * tz - rz * ty - rw * tx;
	double y = -rx * tz + rz * tx - rw * ty;
	double z = rx * ty - ry * tx - rw * tz;
	double w = -rx * tx - ry * ty - rz * tz;

	Transform t;
	t.tx = x * rw + y * rz - z * ry + w * rx;
	t.ty = -x * rz + y * rw + z * rx + w * ry;
	t.tz = x * ry - y * rx + z * rw + w * rz;

	assert(std::isfinite(t.tx));
	assert(std::isfinite(t.ty));
	assert(std::isfinite(t.tz));

	t.rx = -rx;
	t.ry = -ry;
	t.rz = -rz;
	t.rw = rw;

	assert(std::isfinite(t.rx));
	assert(std::isfinite(t.ry));
	assert(std::isfinite(t.rz));
	assert(std::isfinite(t.rw));

	return t;
}

/**
 * Transform point by the inverse of this transform, replacing point with
 * result
 *
 * @param point
 *            point to transform
 */
void Transform::inverseTransformPoint(Vec3 & point) const {
	double px = point.getX() - tx;
	double py = point.getY() - ty;
	double pz = point.getZ() - tz;

	double x = -ry * pz + rz * py + rw * px;
	double y = rx * pz - rz * px + rw * py;
	double z = -rx * py + ry * px + rw * pz;
	double w = rx * px + ry * py + rz * pz;

	point.set(x * rw + y * rz - z * ry + w * rx,
			-x * rz + y * rw + z * rx + w * ry,
			x * ry - y * rx + z * rw + w * rz);
}

/**
 * Transform normal by this transform, replacing normal with result
 *
 * @param normal
 *            point to transform
 */
void Transform::rotate(Normal & normal) const {
	float nx = normal.getX();
	float ny = normal.getY();
	float nz = normal.getZ();

	float x = ry * nz - rz * ny + rw * nx;
	float y = -rx * nz + rz * nx + rw * ny;
	float z = rx * ny - ry * nx + rw * nz;
	float w = -rx * nx - ry * ny - rz * nz;

	normal.set(x * rw - y * rz + z * ry - w * rx,
			x * rz + y * rw - z * rx - w * ry,
			-x * ry + y * rx + z * rw - w * rz);
}

/**
 * Rotate by quaternion, and normalize
 *
 * T = T
 * R = R * r
 *
 * @param r
 * 			rotation to apply
 */
void Transform::rotate(const Quat & r) {
	float x = rx * r.getW() + ry * r.getZ() - rz * r.getY() + rw * r.getX();
	float y = -rx * r.getZ() + ry * r.getW() + rz * r.getX() + rw * r.getY();
	float z = rx * r.getY() - ry * r.getX() + rz * r.getW() + rw * r.getZ();
	float w = -rx * r.getX() - ry * r.getY() - rz * r.getZ() + rw * r.getW();
	float l = 1 / std::sqrt(x * x + y * y + z * z + w * w);
	assert(std::isfinite(l));
	rx = x * l;
	ry = y * l;
	rz = z * l;
	rw = w * l;

	assert(std::isfinite(rx));
	assert(std::isfinite(ry));
	assert(std::isfinite(rz));
	assert(std::isfinite(rw));
}

/**
 * Transform normal by the inverse of this transform, replacing normal with
 * result
 *
 * @param normal
 *            point to transform
 */
void Transform::rotateInverse(Normal & normal) const {
	float nx = normal.getX();
	float ny = normal.getY();
	float nz = normal.getZ();

	float x = -ry * nz + rz * ny + rw * nx;
	float y = rx * nz - rz * nx + rw * ny;
	float z = -rx * ny + ry * nx + rw * nz;
	float w = rx * nx + ry * ny + rz * nz;

	normal.set(x * rw + y * rz - z * ry + w * rx,
			-x * rz + y * rw + z * rx + w * ry,
			x * ry - y * rx + z * rw + w * rz);
}

/**
 * Set rotation to angle about axis
 *
 * @param axis
 *            rotation axis
 * @param angle
 *            rotation angle
 */
void Transform::setRotation(const Vec3 & axis, float angle) {
	float s = std::sin(angle / 2);
	rx = static_cast<float>(axis.getX() * s);
	ry = static_cast<float>(axis.getY() * s);
	rz = static_cast<float>(axis.getZ() * s);
	rw = std::cos(angle / 2);

	assert(std::isfinite(rx));
	assert(std::isfinite(ry));
	assert(std::isfinite(rz));
	assert(std::isfinite(rw));
}

/**
 * Calculate the transform that will take an object from 'this' space to
 * 'other' space
 *
 * @return transform that takes objects from 'this' space into 'other' space
 *         inverse( 'other' ) * 'this'
 */
Transform Transform::to(const Transform & other) const {
	Transform t;
	t.tx = tx - other.tx;
	t.ty = ty - other.ty;
	t.tz = tz - other.tz;

	double x = -other.ry * t.tz + other.rz * t.ty + other.rw * t.tx;
	double y = other.rx * t.tz - other.rz * t.tx + other.rw * t.ty;
	double z = -other.rx * t.ty + other.ry * t.tx + other.rw * t.tz;
	double w = other.rx * t.tx + other.ry * t.ty + other.rz * t.tz;

	t.tx = x * other.rw + y * other.rz - z * other.ry + w * other.rx;
	t.ty = -x * other.rz + y * other.rw + z * other.rx + w * other.ry;
	t.tz = x * other.ry - y * other.rx + z * other.rw + w * other.rz;

	assert(std::isfinite(t.tx));
	assert(std::isfinite(t.ty));
	assert(std::isfinite(t.tz));

	t.rx = -other.rx * rw - other.ry * rz + other.rz * ry + other.rw * rx;
	t.ry = other.rx * rz - other.ry * rw - other.rz * rx + other.rw * ry;
	t.rz = -other.rx * ry + other.ry * rx - other.rz * rw + other.rw * rz;
	t.rw = other.rx * rx + other.ry * ry + other.rz * rz + other.rw * rw;

	assert(std::isfinite(t.rx));
	assert(std::isfinite(t.ry));
	assert(std::isfinite(t.rz));
	assert(std::isfinite(t.rw));

	return t;
}

/**
 *
 *
 * @return transform as string
 */
std::string Transform::toString() const {
	return "(" + std::to_string(tx) + ", " + std::to_string(ty) + ", "
			+ std::to_string(tz) + ", " + std::to_string(rx) + ", "
			+ std::to_string(ry) + ", " + std::to_string(rz) + ", "
			+ std::to_string(rw) + ")";
}

/**
 * transform 'other' by 'this', replacing 'this'
 *
 * T = T + T * R * T'
 * R = R * R'
 *
 * @param other
 */
void Transform::transform(const Transform & other) {
	double x = ry * other.tz - rz * other.ty + rw * other.tx;
	double y = -rx * other.tz + rz * other.tx + rw * other.ty;
	double z = rx * other.ty - ry * other.tx + rw * other.tz;
	double w = -rx * other.tx - ry * other.ty - rz * other.tz;

	tx += x * rw - y * rz + z * ry - w * rx;
	ty += x * rz + y * rw - z * rx - w * ry;
	tz += -x * ry + y * rx + z * rw - w * rz;

	assert(std::isfinite(tx));
	assert(std::isfinite(ty));
	assert(std::isfinite(tz));

	float a, b, c, d;
	a = rx * other.rw + ry * other.rz - rz * other.ry + rw * other.rx;
	b = -rx * other.rz + ry * other.rw + rz * other.rx + rw * other.ry;
	c = rx * other.ry - ry * other.rx + rz * other.rw + rw * other.rz;
	d = -rx * other.rx - ry * other.ry - rz * other.rz + rw * other.rw;
	rx = a;
	ry = b;
	rz = c;
	rw = d;

	assert(std::isfinite(rx));
	assert(std::isfinite(ry));
	assert(std::isfinite(rz));
	assert(std::isfinite(rw));

}

/**
 * replace point with transformed point
 *
 * p = T * R * p
 *
 * @param point
 */
void Transform::transformPoint(Vec3 & point) const {
	double x = ry * point.getZ() - rz * point.getY() + rw * point.getX();
	double y = -rx * point.getZ() + rz * point.getX() + rw * point.getY();
	double z = rx * point.getY() - ry * point.getX() + rw * point.getZ();
	double w = -rx * point.getX() - ry * point.getY() - rz * point.getZ();

	point.set(x * rw - y * rz + z * ry - w * rx + tx,
			x * rz + y * rw - z * rx - w * ry + ty,
			-x * ry + y * rx + z * rw - w * rz + tz);
}

/**
 * Add translation to transform
 *
 * T = T + inv(R) * t
 * R = R
 *
 * @param t
 *            translation to be applied
 */
void Transform::translate(const Vec3 & t) {
	double x = ry * t.getZ() - rz * t.getY() + rw * t.getX();
	double y = -rx * t.getZ() + rz * t.getX() + rw * t.getY();
	double z = rx * t.getY() - ry * t.getX() + rw * t.getZ();
	double w = -rx * t.getX() - ry * t.getY() - rz * t.getZ();

	tx += x * rw - y * rz + z * ry - w * rx;
	ty += x * rz + y * rw - z * rx - w * ry;
	tz += -x * ry + y * rx + z * rw - w * rz;

	assert(std::isfinite(tx));
	assert(std::isfinite(ty));
	assert(std::isfinite(tz));
}

/**
 * get script object factory for Transform
 *
 * @return  Transform factory
 */
STATIC ScriptObjectPtr Transform::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
