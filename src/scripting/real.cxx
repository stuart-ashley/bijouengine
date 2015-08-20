#include "real.h"

#include "bool.h"
#include "executable.h"
#include "parameters.h"
#include "scriptExecutionException.h"
#include "string.h"

#include <cassert>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <unordered_map>

namespace {

	class Add: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(x + y));
		}
	};

	class Sub: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(x - y));
		}
	};

	class Mul: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(x * y));
		}
	};

	class Div: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(x / y));
		}
	};

	class Pow: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			stack.emplace(std::make_shared<Real>(std::pow(x, y)));
		}
	};

	class Lt: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			if (x < y) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class Lte: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			if (x <= y) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class Gt: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			if (x > y) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class Gte: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			double x = std::static_pointer_cast<Real>(self)->getValue();
			double y = getNumericArg(stack, 1);

			if (x >= y) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class Neg: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			double x = std::static_pointer_cast<Real>(self)->getValue();

			stack.emplace(std::make_shared<Real>(-x));
		}
	};

	class Format: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			double value = std::static_pointer_cast<Real>(self)->getValue();

			int a = getInt32Arg(stack, 1);
			int b = getInt32Arg(stack, 2);

			std::ostringstream s;
			if (std::isfinite(value)) {
				s << std::setw(a) << std::fixed << std::setprecision(b)
						<< value;
			} else {
				s << value;
			}

			stack.emplace(std::make_shared<String>(s.str()));
		}
	};

}

/**
 * destructor
 */
Real::~Real() {
}

/**
 * is this script object equal to other script object
 *
 * @param other  script object to compare against
 *
 * @return       true if equal, false otherwise
 */
OVERRIDE bool Real::equals(const ScriptObjectPtr & other) const {
	if (typeid(*other) != typeid(Real)) {
		return false;
	}
	return m_value == std::static_pointer_cast<Real>(other)->m_value;
}

/*
 *
 */
float Real::getFloat() const {
	return static_cast<float>(m_value);
}

/*
 *
 */
int Real::getInt32() const {
	assert(isInt32());
	return static_cast<int>(m_value);
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Real::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__add__", std::make_shared<Add>() },
			{ "__sub__", std::make_shared<Sub>() },
			{ "__mul__", std::make_shared<Mul>() },
			{ "__div__", std::make_shared<Div>() },
			{ "__pow__", std::make_shared<Pow>() },
			{ "__lt__",	std::make_shared<Lt>() },
			{ "__lte__", std::make_shared<Lte>() },
			{ "__gt__", std::make_shared<Gt>() },
			{ "__gte__", std::make_shared<Gte>() },
			{ "__neg__", std::make_shared<Neg>() },
			{ "format",	std::make_shared<Format>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/*
 *
 */
short Real::getInt16() const {
	assert(isInt16());
	return static_cast<short>(m_value);
}

/*
 *
 */
bool Real::isInt32() const {
	if (std::floor(m_value) != m_value) {
		return false;
	}
	if (m_value < std::numeric_limits<int>::min()) {
		return false;
	}
	if (m_value > std::numeric_limits<int>::max()) {
		return false;
	}
	return true;
}

/*
 *
 */
bool Real::isInt16() const {
	if (std::floor(m_value) != m_value) {
		return false;
	}
	if (m_value < std::numeric_limits<short>::min()) {
		return false;
	}
	if (m_value > std::numeric_limits<short>::max()) {
		return false;
	}
	return true;
}
