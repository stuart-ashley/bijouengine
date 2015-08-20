#include "quat.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/real.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <stack>

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 4);

			auto w = getFloatArg(stack, 1);
			auto x = getFloatArg(stack, 2);
			auto y = getFloatArg(stack, 3);
			auto z = getFloatArg(stack, 4);

			stack.emplace(std::make_shared<Quat>(w, x, y, z));
		}
	};

	/*
	 *
	 */
	class GetConjugate: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto quat = std::static_pointer_cast<Quat>(self);

			stack.emplace(std::make_shared<Quat>(quat->conjugate()));
		}
	};

	/*
	 *
	 */
	class Mul: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto quatPtr = std::static_pointer_cast<Quat>(self);

			auto arg = stack.top();
			stack.pop();

			if (typeid(*arg) == typeid(Quat)) {
				auto other = *std::static_pointer_cast<Quat>(arg);
				stack.emplace(std::make_shared<Quat>(*quatPtr * other));
				return;
			} else if (typeid(*arg) == typeid(Vec3)) {
				auto v = quatPtr->rotate(*std::static_pointer_cast<Vec3>(arg));
				stack.emplace(std::make_shared<Vec3>(v));
			} else if (typeid(*arg) == typeid(Normal)) {
				Normal n = quatPtr->rotate(*std::static_pointer_cast<Normal>(arg));
				stack.emplace(std::make_shared<Normal>(n));
			} else {
				scriptExecutionAssert(false,
						"Require quat, vec3 or normal argument");
			}
		}
	};
}
/**
 * construct quaternion from pitch & yaw
 *
 * @param pitch
 * @param yaw
 */
Quat::Quat(float pitch, float yaw) {
	set(pitch, yaw);
}

/**
 * construct quaternion from axis and angle
 *
 * @param axis   axis for quaternion
 * @param angle  angle for quaternion
 */
Quat::Quat(const Normal & axis, float angle) {
	if (axis.invalid()) {
		m_w = 0.0f;
		m_x = 0.0f;
		m_y = 0.0f;
		m_z = 0.0f;
	} else {
		float s = std::sin(angle / 2);
		m_w = std::cos(angle / 2);
		m_x = axis.getX() * s;
		m_y = axis.getY() * s;
		m_z = axis.getZ() * s;
	}
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Quat::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{"getConjugate", std::make_shared<GetConjugate>() },
			{ "__mul__", std::make_shared<Mul>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/*
 *
 */
void Quat::interpolate(const Quat & q, const Quat & r, float t) {
	float dot = q.m_x * r.m_x + q.m_y * r.m_y + q.m_z * r.m_z + q.m_w * r.m_w;

	// reverse r to reduce spin
	if (dot < 0) {
		if (dot < -0.95f) {
			m_x = q.m_x * (1 - t) - r.m_x * t;
			m_y = q.m_y * (1 - t) - r.m_y * t;
			m_z = q.m_z * (1 - t) - r.m_z * t;
			m_w = q.m_w * (1 - t) - r.m_w * t;
			return;
		}

		float angle = std::acos(dot);
		float a = std::sin(angle * (1 - t));
		float b = -std::sin(angle * t);
		float c = 1 / std::sin(angle);
		m_x = (q.m_x * a - r.m_x * b) * c;
		m_y = (q.m_y * a - r.m_y * b) * c;
		m_z = (q.m_z * a - r.m_z * b) * c;
		m_w = (q.m_w * a - r.m_w * b) * c;
		return;
	}

	if (dot > 0.95f) {
		m_x = q.m_x * (1 - t) + r.m_x * t;
		m_y = q.m_y * (1 - t) + r.m_y * t;
		m_z = q.m_z * (1 - t) + r.m_z * t;
		m_w = q.m_w * (1 - t) + r.m_w * t;
		return;
	}

	float angle = std::acos(dot);
	float a = std::sin(angle * (1 - t));
	float b = std::sin(angle * t);
	float c = 1 / std::sin(angle);
	m_x = (q.m_x * a + r.m_x * b) * c;
	m_y = (q.m_y * a + r.m_y * b) * c;
	m_z = (q.m_z * a + r.m_z * b) * c;
	m_w = (q.m_w * a + r.m_w * b) * c;
}

/**
 * quaternion multiple
 *
 * @param r  right hand quaternion
 *
 * @return   this multiplied by r
 */
Quat Quat::operator*(const Quat & r) const {
	float qx = m_x * r.m_w + m_y * r.m_z - m_z * r.m_y + m_w * r.m_x;
	float qy = -m_x * r.m_z + m_y * r.m_w + m_z * r.m_x + m_w * r.m_y;
	float qz = m_x * r.m_y - m_y * r.m_x + m_z * r.m_w + m_w * r.m_z;
	float qw = -m_x * r.m_x - m_y * r.m_y - m_z * r.m_z + m_w * r.m_w;

	assert(std::isfinite(qx));
	assert(std::isfinite(qy));
	assert(std::isfinite(qz));
	assert(std::isfinite(qw));

	return Quat(qx, qy, qz, qw);
}

/**
 * in place quaternion multiple
 *
 * @param other  quaternion
 *
 * @return       reference to this
 */
Quat & Quat::operator*=(const Quat & r) {
	float qx = m_x * r.m_w + m_y * r.m_z - m_z * r.m_y + m_w * r.m_x;
	float qy = -m_x * r.m_z + m_y * r.m_w + m_z * r.m_x + m_w * r.m_y;
	float qz = m_x * r.m_y - m_y * r.m_x + m_z * r.m_w + m_w * r.m_z;
	float qw = -m_x * r.m_x - m_y * r.m_y - m_z * r.m_z + m_w * r.m_w;

	assert(std::isfinite(qx));
	assert(std::isfinite(qy));
	assert(std::isfinite(qz));
	assert(std::isfinite(qw));

	m_x = qx;
	m_y = qy;
	m_z = qz;
	m_w = qw;

	return *this;
}

/**
 * rotate normal by quaternion
 *
 * n' = q * n * conj(q)
 *
 * @param n  normal to rotate
 *
 * @return   post rotation normal
 */
Normal Quat::rotate(const Normal & n) const {
	float nx = n.getX();
	float ny = n.getY();
	float nz = n.getZ();

	float tx = m_y * nz - m_z * ny + m_w * nx;
	float ty = -m_x * nz + m_z * nx + m_w * ny;
	float tz = m_x * ny - m_y * nx + m_w * nz;
	float tw = -m_x * nx - m_y * ny - m_z * nz;

	return Normal(tx * m_w - ty * m_z + tz * m_y - tw * m_x,
			tx * m_z + ty * m_w - tz * m_x - tw * m_y,
			-tx * m_y + ty * m_x + tz * m_w - tw * m_z);
}

/**
 * rotate vector by quaternion
 *
 * v' = q * v * conj(q)
 *
 * @param v  vector to rotate
 *
 * @return   post rotation vector
 */
Vec3 Quat::rotate(const Vec3 & v) const {
	float vx = static_cast<float>(v.getX());
	float vy = static_cast<float>(v.getY());
	float vz = static_cast<float>(v.getZ());

	double tx = m_y * vz - m_z * vy + m_w * vx;
	double ty = -m_x * vz + m_z * vx + m_w * vy;
	double tz = m_x * vy - m_y * vx + m_w * vz;
	double tw = -m_x * vx - m_y * vy - m_z * vz;

	return Vec3(tx * m_w - ty * m_z + tz * m_y - tw * m_x,
			tx * m_z + ty * m_w - tz * m_x - tw * m_y,
			-tx * m_y + ty * m_x + tz * m_w - tw * m_z);
}

/**
 * rotate Vec3( sx, 0, 0 ) by this rotation
 */
Vec3 Quat::rotateX(double sx) {
	double s = 2 * sx;
	double vx = sx - s * (m_y * m_y + m_z * m_z);
	double vy = s * (m_x * m_y + m_w * m_z);
	double vz = s * (m_x * m_z - m_w * m_y);

	return Vec3(vx, vy, vz);
}

/**
 * rotate Vec3( 0, sy, 0 ) by this rotation
 */
Vec3 Quat::rotateY(double sy) {
	double s = 2 * sy;
	double vx = s * (m_x * m_y - m_w * m_z);
	double vy = sy - s * (m_x * m_x + m_z * m_z);
	double vz = s * (m_y * m_z + m_w * m_x);

	return Vec3(vx, vy, vz);
}

/**
 * rotate Vec3( 0, 0, sz ) by this rotation
 */
Vec3 Quat::rotateZ(double sz) {
	double s = 2 * sz;
	double vx = s * (m_x * m_z + m_w * m_y);
	double vy = s * (m_y * m_z - m_w * m_x);
	double vz = sz - s * (m_x * m_x + m_y * m_y);

	return Vec3(vx, vy, vz);
}

/**
 * set quaternion from pitch & yaw
 *
 * @param pitch
 * @param yaw
 */
void Quat::set(float pitch, float yaw) {
	float yw = std::cos(yaw / 2);
	float yz = std::sin(yaw / 2);

	float pw = std::cos(pitch / 2);
	float px = std::sin(pitch / 2);

	m_x = yw * px;
	m_y = yz * px;
	m_z = yz * pw;
	m_w = yw * pw;
}

/**
 * set quaternion and normalize
 */
void Quat::set(float x, float y, float z, float w) {
	float scale = 1 / std::sqrt(x * x + y * y + z * z + w * w);

	m_x = x * scale;
	m_y = y * scale;
	m_z = z * scale;
	m_w = w * scale;
}

/**
 * get script object factory for Quat
 *
 * @return  Quat factory
 */
STATIC const ScriptObjectPtr & Quat::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
