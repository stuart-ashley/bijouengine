#pragma once

#include "../scripting/scriptObject.h"

#include <string>

class ScriptExecutionState;

class CoreModule: public ScriptObject {
public:
	CoreModule(const std::string & currentDir);

	~CoreModule();

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
	struct impl;
	std::unique_ptr<impl> pimpl;
};



