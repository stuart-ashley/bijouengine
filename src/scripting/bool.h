#pragma once

#include <memory>
#include <string>
#include "scriptObject.h"

class ScriptExecutionState;

class Bool: public ScriptObject {
public:
	~Bool();

	static const std::shared_ptr<Bool> & True();

	static const std::shared_ptr<Bool> & False();

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
	 * calculate string representation of Bool
	 *
	 * @return  Bool as string
	 */
	inline std::string toString() const override {
		if (this == True().get()) {
			return "true";
		} else {
			return "false";
		}
	}

private:
	Bool();
};
