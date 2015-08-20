#include "color.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"

#include <unordered_map>

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {
		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			if (nArgs == 3) {
				auto r = getFloatArg(stack, 1);
				auto g = getFloatArg(stack, 2);
				auto b = getFloatArg(stack, 3);

				stack.emplace(std::make_shared<Color>(r, g, b));
			} else if (nArgs == 4) {
				auto r = getFloatArg(stack, 1);
				auto g = getFloatArg(stack, 2);
				auto b = getFloatArg(stack, 3);
				auto a = getFloatArg(stack, 4);

				stack.emplace(std::make_shared<Color>(r, g, b, a));
			} else {
				scriptExecutionAssert(false,
						"Require 3 or 4 argument got " + std::to_string(nArgs));
			}
		}
	};

	/*
	 *
	 */
	class Add: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto color = std::static_pointer_cast<Color>(self);

			auto other = getArg<Color>("Color", stack, 1);

			stack.emplace(std::make_shared<Color>(*color + other));
		}
	};

	/*
	 *
	 */
	class Sub: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto color = std::static_pointer_cast<Color>(self);

			auto other = getArg<Color>("Color", stack, 1);

			stack.emplace(std::make_shared<Color>(*color - other));
		}
	};

	/*
	 *
	 */
	class Mul: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto color = std::static_pointer_cast<Color>(self);

			auto scale = static_cast<float>(getNumericArg(stack, 1));

			stack.emplace(std::make_shared<Color>(*color * scale));
		}
	};

	/*
	 *
	 */
	class Div: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto color = std::static_pointer_cast<Color>(self);

			auto scale = static_cast<float>(getNumericArg(stack, 1));

			stack.emplace(std::make_shared<Color>(*color / scale));
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
OVERRIDE ScriptObjectPtr Color::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__add__", std::make_shared<Add>() },
			{ "__sub__", std::make_shared<Sub>() },
			{ "__mul__", std::make_shared<Mul>() },
			{ "__div__", std::make_shared<Div>() } };
			
	if (name == "r") {
		return std::make_shared<Real>(r);
	} else if (name == "g") {
		return std::make_shared<Real>(g);
	} else if (name == "b") {
		return std::make_shared<Real>(b);
	} else if (name == "a") {
		return std::make_shared<Real>(a);
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
OVERRIDE std::string Color::toString() const {
	return "Color(" + std::to_string(r) + ", " + std::to_string(g) + ", "
			+ std::to_string(b) + ", " + std::to_string(a) + ")";
}
/*
 *
 */
STATIC Color Color::black() {
	return Color(0, 0, 0);
}

/*
 *
 */
STATIC Color Color::blue() {
	return Color(0, 0, 1);
}

/*
 *
 */
STATIC Color Color::green() {
	return Color(0, 1, 0);
}

/*
 *
 */
STATIC Color Color::red() {
	return Color(1, 0, 0);
}

/*
 *
 */
STATIC Color Color::white() {
	return Color(1, 1, 1);
}

/**
 * get script object factory for Color
 *
 * @return  Color factory
 */
STATIC const ScriptObjectPtr & Color::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
