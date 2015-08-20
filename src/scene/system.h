#pragma once

#include "../scripting/scriptObject.h"

#include <string>
#include <unordered_map>

class System: public ScriptObject {
public:

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	/**
	 * set named script object member
	 *
	 * @param name   name of member
	 * @param value  desired value
	 */
	void setMember(const std::string & name, const ScriptObjectPtr & value)
			override;

private:
	std::unordered_map<std::string, ScriptObjectPtr> members;
};

