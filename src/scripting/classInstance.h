#pragma once

#include "scriptObject.h"

#include <memory>
#include <string>
#include <unordered_map>

class ScriptExecutionState;

class ClassInstance: public ScriptObject {
public:
	ClassInstance(const std::string & className);

	~ClassInstance();

	const std::unordered_map<std::string, ScriptObjectPtr> & getMembers() const;

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
	void setMember(const std::string & name, const ScriptObjectPtr & value) override;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

