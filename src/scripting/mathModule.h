#pragma once

#include "scriptObject.h"

#include <string>

class ScriptExecutionState;

class MathModule final: public ScriptObject {
public:
	/**
	 * default destructor
	 */
	inline virtual ~MathModule() = default;

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
};
