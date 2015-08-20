#include "bool.h"

#include "executable.h"
#include "parameters.h"
#include "scriptExecutionException.h"

namespace {
	class BoolEqBool: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			scriptExecutionAssertType<Bool>(stack.top(),
					"Require boolean value");

			if (self == Bool::False()) {
				auto other = stack.top();
				stack.pop();
				if (other == Bool::True()) {
					stack.emplace(Bool::False());
				} else {
					stack.emplace(Bool::True());
				}
			}
		}
	};

	class BoolNeqBool: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			scriptExecutionAssertType<Bool>(stack.top(),
					"Require boolean value");

			if (self == Bool::True()) {
				auto other = stack.top();
				stack.pop();
				if (other == Bool::True()) {
					stack.emplace(Bool::False());
				} else {
					stack.emplace(Bool::True());
				}
			}
		}
	};

	class BoolAndBool: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			scriptExecutionAssertType<Bool>(stack.top(),
					"Require boolean value");

			if (self == Bool::False() && stack.top() == Bool::True()) {
				stack.pop();
				stack.emplace(Bool::False());
			}
		}
	};

	class BoolOrBool: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			scriptExecutionAssertType<Bool>(stack.top(),
					"Require boolean value");

			if (self == Bool::True() && stack.top() == Bool::False()) {
				stack.pop();
				stack.emplace(Bool::True());
			}
		}
	};

	class BoolNot: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			if (self == Bool::True()) {
				stack.emplace(Bool::False());
			} else {
				stack.emplace(Bool::True());
			}
		}
	};

}

/*
 *
 */
Bool::Bool() {
}

/*
 *
 */
Bool::~Bool() {
}

/*
 *
 */
const std::shared_ptr<Bool> & Bool::True() {
	static std::shared_ptr<Bool> TRUE(new Bool());
	return TRUE;
}

/*
 *
 */
const std::shared_ptr<Bool> & Bool::False() {
	static std::shared_ptr<Bool> FALSE(new Bool());
	return FALSE;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr Bool::getMember(ScriptExecutionState &,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__eq__", std::make_shared<BoolEqBool>() },
			{ "__neq__", std::make_shared<BoolNeqBool>() },
			{ "__and__", std::make_shared<BoolAndBool>() },
			{ "__or__", std::make_shared<BoolOrBool>() },
			{ "__not__", std::make_shared<BoolNot>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	throw ScriptExecutionException("Can't get member '" + name + "'");
}
