#pragma once

#include "../scripting/scriptObject.h"

#include <string>
#include <unordered_map>

class ScriptExecutionState;

class SceneModule: public ScriptObject {
public:
	SceneModule(const std::string & currentDir);

	~SceneModule();

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
private:
	std::unordered_map<std::string, ScriptObjectPtr> members;
};

