#include "inputEvent.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

namespace {

	/*
	 *
	 */
	class GetAction: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto inputEvent = std::static_pointer_cast<InputEvent>(self);

			stack.emplace(std::make_shared<String>(inputEvent->getAction()));
		}
	};

	/*
	 *
	 */
	class GetData: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto inputEvent = std::static_pointer_cast<InputEvent>(self);

			stack.emplace(inputEvent->getData()	);
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
ScriptObjectPtr InputEvent::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{"getAction", std::make_shared<GetAction>() },
			{ "getData", std::make_shared<GetData>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}
