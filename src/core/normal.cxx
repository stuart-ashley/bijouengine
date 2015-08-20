#include "normal.h"

#include "vec3.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"

#include <cmath>
#include <unordered_map>

namespace {
	float epsilon = .001f;

	/*
	 *
	 */
	struct Factory: public Executable {
		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 3);

			auto x = static_cast<float>(getNumericArg(stack, 1));
			auto y = static_cast<float>(getNumericArg(stack, 2));
			auto z = static_cast<float>(getNumericArg(stack, 3));

			stack.emplace(std::make_shared<Normal>(x, y, z));
		}
	};

	/*
	 *
	 */
	class Neg: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & normal = std::static_pointer_cast<Normal>(self);
			auto result = std::make_shared<Normal>(-(*normal));
			stack.emplace(result);
		}
	};

	/*
	 *
	 */
	class Dot: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			const auto & normal = *std::static_pointer_cast<Normal>(self);

			auto e = stack.top();
			stack.pop();
			if (typeid(*e) == typeid(Vec3)) {
				auto v = std::static_pointer_cast<Vec3>(e);
				stack.emplace(std::make_shared<Real>(v->dot(normal)));
			} else if (typeid(*e) == typeid(Normal)) {
				auto n = std::static_pointer_cast<Normal>(e);
				stack.emplace(std::make_shared<Real>(n->dot(normal)));
			} else {
				scriptExecutionAssert(false,
						"Require Vec3 or Normal for dot product");
			}
		}
	};
}

Normal::Normal(double x, double y, double z) {
	auto l2 = x * x + y * y + z * z;
	if (l2 > -epsilon && l2 < epsilon) {
		m_x = 0;
		m_y = 0;
		m_z = 0;
	} else {
		auto s = 1.f / std::sqrt(l2);
		m_x = static_cast<float>(x * s);
		m_y = static_cast<float>(y * s);
		m_z = static_cast<float>(z * s);
	}
}

Normal::Normal(float x, float y, float z) {
	auto l2 = x * x + y * y + z * z;
	if (l2 > -epsilon && l2 < epsilon) {
		m_x = 0;
		m_y = 0;
		m_z = 0;
	} else {
		auto s = 1.f / std::sqrt(l2);
		m_x = x * s;
		m_y = y * s;
		m_z = z * s;
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
OVERRIDE ScriptObjectPtr Normal::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__neg__", std::make_shared<Neg>() },
			{ "dot", std::make_shared<Dot>() } };

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

/*
 *
 */
void Normal::set(float x, float y, float z) {
	auto l2 = x * x + y * y + z * z;
	if (l2 > -epsilon && l2 < epsilon) {
		m_x = 0;
		m_y = 0;
		m_z = 0;
	} else {
		auto s = 1.f / std::sqrt(l2);
		m_x = x * s;
		m_y = y * s;
		m_z = z * s;
	}
}

/**
 * get script object factory for Normal
 *
 * @return  Normal factory
 */
STATIC const ScriptObjectPtr & Normal::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
