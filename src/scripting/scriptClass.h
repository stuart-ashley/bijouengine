#pragma once

#include "scriptObject.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Procedure;
class BreakpointMarker;

class ScriptClass: public ScriptObject {
public:

	/**
	 *
	 * @param className
	 * @param members
	 * @param initProc
	 * @param instanceFunctions
	 */
	ScriptClass(const std::string & className,
			const std::unordered_map<std::string, ScriptObjectPtr> & members,
			const std::shared_ptr<Procedure> & initProc,
			const std::unordered_map<std::string, std::shared_ptr<Procedure> > & instanceFunctions);

	/**
	 * destructor
	 */
	~ScriptClass();

	/**
	 *
	 * @return
	 */
	std::vector<std::shared_ptr<BreakpointMarker> > getBreakpoints() const;

	/**
	 *
	 * @return
	 */
	const std::string & getClassName() const;

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
	 *
	 * @param execState
	 */
	void init(ScriptExecutionState & execState);

	/**
	 *
	 * @return
	 */
	ScriptObjectPtr newInstance() const;

	/**
	 * set named script object member
	 *
	 * @param name   name of member
	 * @param value  desired value
	 */
	void setMember(const std::string & name, const ScriptObjectPtr & value)
			override;

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

