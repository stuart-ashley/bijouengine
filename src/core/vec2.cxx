#include "vec2.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/real.h"

#include <cstdio>
#include <cmath>
#include <unordered_map>

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 2);

			auto x = getFloatArg(stack,1);
			auto y = getFloatArg(stack,2);

			stack.emplace(std::make_shared<Vec2>(x, y));
		}
	};

	/*
	 *
	 */
	class Sub: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto vec2Ptr = std::static_pointer_cast<Vec2>(self);

			auto other = getArg<Vec2>("Vec2", stack, 1);

			stack.emplace(std::make_shared<Vec2>(*vec2Ptr - other));
		}
	};
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Vec2::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{"__sub__", std::make_shared<Sub>() } };

	if (name == "x") {
		return std::make_shared<Real>(m_x);
	} else if (name == "y") {
		return std::make_shared<Real>(m_y);
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
float Vec2::length() const {
	return sqrtf(dot(*this));
}

/*
 *
 */
void Vec2::dump() const {
	printf("( %f, %f )\n", m_x, m_y);
}

/**
 * get script object factory for Vec2
 *
 * @return  Vec2 factory
 */
STATIC const ScriptObjectPtr & Vec2::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
