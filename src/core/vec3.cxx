#include "vec3.h"

#include "mat3.h"
#include "quat.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_map>

namespace {
	/*
	 *
	 */
	class Factory: public Executable {
	public:
		Factory() {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 3);

			double x = getNumericArg(stack,1);
			double y = getNumericArg(stack,2);
			double z = getNumericArg(stack,3);

			stack.emplace(std::make_shared<Vec3>(x, y, z));
		}
	};

	/*
	 *
	 */
	class Add: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			auto other = getArg<Vec3>("vec3", stack, 1);

			stack.emplace(std::make_shared<Vec3>(*vec3Ptr + other));
		}
	};

	/*
	 *
	 */
	class Cross: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			auto arg = stack.top();
			stack.pop();

			if (typeid(*arg) == typeid(Vec3)) {
				auto v = *std::static_pointer_cast<Vec3>(arg);

				stack.emplace(std::make_shared<Vec3>(vec3Ptr->cross(v)));
			} else if (typeid(*arg) == typeid(Normal)) {
				auto n = *std::static_pointer_cast<Normal>(arg);

				stack.emplace(std::make_shared<Vec3>(vec3Ptr->cross(n)));
			} else {
				scriptExecutionAssert(false,
						"Require Vec3 or Normal as argument to cross");
			}
		}
	};

	/*
	 *
	 */
	class Div: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			auto scale = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Vec3>(*vec3Ptr / scale));
		}
	};

	/*
	 *
	 */
	class Dot: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			auto other = getArg<Vec3>("vec3", stack, 1);

			stack.emplace(std::make_shared<Real>(vec3Ptr->dot(other)));
		}
	};

	/*
	 *
	 */
	struct LookAt: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto target = getArg<Vec3>("Vec3", stack, 1);
			auto vec = *std::static_pointer_cast<Vec3>(self) - target;

			Normal fwd(0.f, 1.f, 0.f);
			if (vec.length() != 0) {
				fwd = vec.normalized();
			}
			Normal up(0.f, 0.f, 1.f);
			auto right = up.cross(fwd);
			up = fwd.cross(right);

			Mat3 m(right.getX(), up.getX(), fwd.getX(), right.getY(), up.getY(),
					fwd.getY(), right.getZ(), up.getZ(), fwd.getZ());
			auto r = m.asQuat();

			stack.push(std::make_shared<Quat>(r));
		}
	};

	/*
	 *
	 */
	class Magnitude: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			stack.emplace(std::make_shared<Real>(vec3Ptr->length()));
		}
	};

	/*
	 *
	 */
	class Mul: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			auto scale = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Vec3>(*vec3Ptr * scale));
		}
	};

	/*
	 *
	 */
	class Sub: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec3Ptr = std::static_pointer_cast<Vec3>(self);

			auto other = getArg<Vec3>("vec3", stack, 1);

			stack.emplace(std::make_shared<Vec3>(*vec3Ptr - other));
		}
	};
}

Vec3::Vec3(double x, double y, double z) :
		m_x(x), m_y(y), m_z(z) {
	assert(std::isfinite(x));
	assert(std::isfinite(y));
	assert(std::isfinite(z));
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Vec3::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{"__add__", std::make_shared<Add>() },
			{"cross", std::make_shared<Cross>() },
			{"__div__", std::make_shared<Div>() },
			{"dot", std::make_shared<Dot>() },
			{"lookAt", std::make_shared<LookAt>() },
			{"magnitude", std::make_shared<Magnitude>() },
			{"__mul__", std::make_shared<Mul>() },
			{"__sub__", std::make_shared<Sub>() } };

	if (name == "x") {
		return std::make_shared<Real>(m_x);
	} else if (name == "y") {
		return std::make_shared<Real>(m_y);
	} else if (name == "z") {
		return std::make_shared<Real>(m_z);
	}

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * this = u * (1 - t) + v * t
 *
 * @param u
 * @param v
 * @param t
 */
void Vec3::interpolate(const Vec3 & u, const Vec3 & v, double t) {
	assert(std::isfinite(t));
	m_x = u.m_x * (1 - t) + v.m_x * t;
	m_y = u.m_y * (1 - t) + v.m_y * t;
	m_z = u.m_z * (1 - t) + v.m_z * t;
}

/*
 *
 */
double Vec3::length() const {
	return std::sqrt(dot(*this));
}

/*
 *
 */
void Vec3::scale(double s, const Normal & n) {
	assert(std::isfinite(s));
	m_x = n.getX() * s;
	m_y = n.getY() * s;
	m_z = n.getZ() * s;
}

/*
 *
 */
void Vec3::scaleAdd(double s, const Normal & n, const Vec3 & v) {
	assert(std::isfinite(s));
	m_x = n.getX() * s + v.m_x;
	m_y = n.getY() * s + v.m_y;
	m_z = n.getZ() * s + v.m_z;
}

/*
 *
 */
void Vec3::scaleAdd(double s, const Vec3 & u, const Vec3 & v) {
	assert(std::isfinite(s));
	m_x = u.getX() * s + v.m_x;
	m_y = u.getY() * s + v.m_y;
	m_z = u.getZ() * s + v.m_z;
}

/*
 *
 */
void Vec3::scaleAdd(double s, const Vec3 & v) {
	assert(std::isfinite(s));
	m_x = m_x * s + v.m_x;
	m_y = m_y * s + v.m_y;
	m_z = m_z * s + v.m_z;
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
OVERRIDE void Vec3::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	if (name == "x") {
		m_x = std::static_pointer_cast<Real>(value)->getValue();
	} else if (name == "y") {
		m_y = std::static_pointer_cast<Real>(value)->getValue();
	} else if (name == "z") {
		m_z = std::static_pointer_cast<Real>(value)->getValue();
	} else {
		scriptExecutionAssert(false, "Can't set member '" + name + "'");
	}
}

/**
 * get script object factory for Vec3
 *
 * @return  Vec3 factory
 */
STATIC const ScriptObjectPtr & Vec3::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
