#include "mathModule.h"

#include "executable.h"
#include "list.h"
#include "parameters.h"
#include "real.h"
#include "scriptExecutionException.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <random>
#include <unordered_map>

namespace {
	class Rand: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			double r = static_cast<double>(rand()) / RAND_MAX;
			stack.emplace(std::make_shared<Real>(r));
		}
	};

	class Max: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			double a = getNumericArg(stack, 1);
			double b = getNumericArg(stack, 2);

			stack.emplace(std::make_shared<Real>(std::max(a, b)));
		}
	};

	class Min: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			double a = getNumericArg(stack, 1);
			double b = getNumericArg(stack, 2);

			stack.emplace(std::make_shared<Real>(std::min(a, b)));
		}
	};

	class Clamp: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 3);

			double a = getNumericArg(stack, 1);
			double b = getNumericArg(stack, 2);
			double c = getNumericArg(stack, 3);

			if (b > c) {
				// error("clamp error " + b + " > " + c, program,
				// ((Script.Function) ScriptObjectPtr).pos);
			}
			stack.emplace(std::make_shared<Real>(std::min(std::max(a, b), c)));
		}
	};

	class Abs: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::abs(x)));
		}
	};

	class Ceil: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::ceil(x)));
		}
	};

	class Floor: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::floor(x)));
		}
	};

	class Sin: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double angle = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::sin(angle)));
		}
	};

	class Cos: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double angle = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::cos(angle)));
		}
	};

	class Tan: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double angle = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::tan(angle)));
		}
	};

	class Sqrt: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::sqrt(x)));
		}
	};

	class Range: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			if (nArgs == 1) {
				int num = getInt32Arg(stack, 1);

				std::vector<ScriptObjectPtr> list;
				list.reserve(num);

				for (int i = 0; i < num; ++i) {
					list.emplace_back(std::make_shared<Real>(i));
				}

				stack.emplace(std::make_shared<List>(list));
				return;
			}
			if (nArgs == 2) {
				int from = getInt32Arg(stack, 1);
				int to = getInt32Arg(stack, 2);

				std::vector<ScriptObjectPtr> list;
				list.reserve(to - from);

				for (int i = from; i < to; ++i) {
					list.emplace_back(std::make_shared<Real>(i));
				}

				stack.emplace(std::make_shared<List>(list));
				return;
			}
			if (nArgs == 3) {
				int from = getInt32Arg(stack, 1);
				int to = getInt32Arg(stack, 2);
				int step = getInt32Arg(stack, 3);

				std::vector<ScriptObjectPtr> list;
				list.reserve(std::abs((to - from) / step));

				if (step < 0) {
					for (int i = from; i > to; i += step) {
						list.emplace_back(std::make_shared<Real>(i));
					}
				} else {
					for (int i = from; i < to; i += step) {
						list.emplace_back(std::make_shared<Real>(i));
					}
				}
				stack.emplace(std::make_shared<List>(list));
				return;
			}
			scriptExecutionAssert(false,
					"Require 1, 2 or 3 arguments got " + std::to_string(nArgs));
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
OVERRIDE ScriptObjectPtr MathModule::getMember(ScriptExecutionState &,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "pi",	std::make_shared<Real>(3.14159265358979323846) },
			{ "random",	std::make_shared<Rand>() },
			{ "max", std::make_shared<Max>() },
			{ "min", std::make_shared<Min>() },
			{ "clamp", std::make_shared<Clamp>() },
			{ "abs", std::make_shared<Abs>() },
			{ "ceil", std::make_shared<Ceil>() },
			{ "floor", std::make_shared<Floor>() },
			{ "sin", std::make_shared<Sin>() },
			{ "cos", std::make_shared<Cos>() },
			{ "tan", std::make_shared<Tan>() },
			{ "sqrt", std::make_shared<Sqrt>() },
			{ "range", std::make_shared<Range>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return nullptr;
}
