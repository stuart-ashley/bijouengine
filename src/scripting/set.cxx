#include "set.h"

#include "bool.h"
#include "executable.h"
#include "parameters.h"
#include "real.h"
#include "scriptExecutionException.h"

#include <unordered_map>

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			std::unordered_set<ScriptObjectPtr> set;
			for (unsigned i = 0; i < nArgs; ++i) {
				set.insert(stack.top());
				stack.pop();
			}
			stack.emplace(std::make_shared<Set>(set));
		}
	};

	/*
	 *
	 */
	class Contains: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto set = std::static_pointer_cast<Set>(self);

			auto element = stack.top();
			stack.pop();

			if (set->contains(element)) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
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

			auto set = std::static_pointer_cast<Set>(self);

			auto element = stack.top();
			stack.pop();

			set->add(element);
		}
	};

	/*
	 *
	 */
	class Remove: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto set = std::static_pointer_cast<Set>(self);

			auto element = stack.top();
			stack.pop();

			set->remove(element);
		}
	};

	/*
	 *
	 */
	class Size: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto set = std::static_pointer_cast<Set>(self);

			stack.emplace(std::make_shared<Real>(static_cast<double>(set->size())));
		}
	};
}

/*
 *
 */
Set::~Set() {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr Set::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "contains", std::make_shared<Contains>() },
			{ "add", std::make_shared<Add>() },
			{ "remove", std::make_shared<Remove>() },
			{ "size", std::make_shared<Size>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * get script object factory for Set
 *
 * @return  Set factory
 */
const ScriptObjectPtr & Set::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
