#include "string.h"

#include "executable.h"
#include "parameters.h"

namespace {
	class Add: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto string = std::static_pointer_cast<String>(self)->getValue();
			auto arg = stack.top();
			stack.pop();

			stack.emplace(std::make_shared<String>(string + arg->toString()));
		}
	};

	class StartsWith: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto string = std::static_pointer_cast<String>(self)->getValue();
			auto starting = getArg<String>("string", stack, 1).getValue();

			auto endPos = starting.length();

			if (endPos <= string.length()
					&& string.compare(0, endPos, starting) == 0) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class EndsWith: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto string = std::static_pointer_cast<String>(self)->getValue();
			auto ending = getArg<String>("string", stack, 1).getValue();

			if (string.length() >= ending.length()
					&& string.compare(string.length() - ending.length(),
							string.length(), ending) == 0) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};
}

/**
 * destructor
 */
String::~String() {
}

/**
 * is this script object equal to other script object
 *
 * @param other  script object to compare against
 *
 * @return       true if equal, false otherwise
 */
OVERRIDE bool String::equals(const ScriptObjectPtr & other) const {
	if (typeid(*other) != typeid(String)) {
		return false;
	}
	return std::static_pointer_cast<String>(other)->m_string == m_string;
}

/**
 * get hash for script object
 *
 * @return  hash for script object
 */
OVERRIDE size_t String::getHash() const {
	return std::hash<std::string>()(m_string);
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr String::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "__add__", std::make_shared<Add>() },
			{ "endsWith", std::make_shared<EndsWith>() },
			{ "startsWith", std::make_shared<StartsWith>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

