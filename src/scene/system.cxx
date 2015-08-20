#include "system.h"

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr System::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
OVERRIDE void System::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	auto result = members.emplace(name, value);
	if (result.second == false) {
		result.first->second = value;
	}
}
