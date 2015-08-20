#include "scriptObject.h"

#include "bool.h"
#include "executable.h"
#include "parameters.h"
#include "scriptExecutionException.h"
#include "scriptExecutionState.h"

#include <unordered_map>
#include <iostream>

namespace {
	class ObjEqObj: public Executable {
		void execute(const std::shared_ptr<ScriptObject> & self, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 1);

			auto arg = stack.top();
			stack.pop();

			if (self->equals(arg)) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class ObjNeqObj: public Executable {
		void execute(const std::shared_ptr<ScriptObject> & self, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			checkNumArgs(nArgs, 1);

			auto arg = stack.top();
			stack.pop();

			if (self->equals(arg)) {
				stack.emplace(Bool::False());
			} else {
				stack.emplace(Bool::True());
			}
		}
	};

}

/**
 * destructor
 */
VIRTUAL ScriptObject::~ScriptObject() {
}

/**
 * is this script object equal to other script object
 *
 * @param other  script object to compare against
 *
 * @return       true if equal, false otherwise
 */
VIRTUAL bool ScriptObject::equals(const ScriptObjectPtr & other) const {
	return this == other.get();
}

/**
 * get hash for script object
 *
 * @return  hash for script object
 */
VIRTUAL size_t ScriptObject::getHash() const {
	throw ScriptExecutionException("Can't get hash");
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
VIRTUAL ScriptObjectPtr ScriptObject::getMember(ScriptExecutionState &,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__eq__", std::make_shared<ObjEqObj>() },
			{ "__neq__", std::make_shared<ObjNeqObj>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	throw ScriptExecutionException("Can't get member '" + name + "'");
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
VIRTUAL void ScriptObject::setMember(const std::string & name,
		const ScriptObjectPtr &) {
	throw ScriptExecutionException("Can't set member '" + name + "'");
}

/**
 * calculate string representation of ScriptObject
 *
 * @return  ScriptObject as string
 */
VIRTUAL std::string ScriptObject::toString() const {
	return std::string(typeid(*this).name());
}
