#pragma once

#include "scriptObject.h"

#include <string>
#include <istream>
#include <vector>

class BreakpointMarker;

class Program: public ScriptObject {
public:

	/**
	 * constructor
	 */
	Program();

	/**
	 * destructor
	 */
	~Program();

	/**
	 *
	 * @param line
	 * @return
	 */
	std::shared_ptr<BreakpointMarker> getBreakpoint(int line);

	/**
	 * get active breakpoints
	 *
	 * @return list of active breakpoints
	 */
	std::vector<std::shared_ptr<BreakpointMarker> > getBreakpoints();

	/**
	 *
	 * @return
	 */
	const std::string & getFilename() const;

	/**
	 *
	 * @return
	 */
	const std::string & getContent() const;

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
	 * @param name
	 * @return
	 */
	bool hasMember(const std::string & name) const;

	/**
	 *
	 * @param execState
	 */
	void init(ScriptExecutionState & execState);

	/**
	 * set named script object member
	 *
	 * @param name   name of member
	 * @param value  desired value
	 */
	void setMember(const std::string & name, const ScriptObjectPtr & value)
			override;

	/**
	 *
	 * @param module
	 */
	void use(const ScriptObjectPtr & module);

	/**
	 *
	 * @param filename
	 * @param stream
	 * @return
	 */
	static std::shared_ptr<Program> create(const std::string & filename,
			std::istream & stream);

private:

	struct impl;
	std::unique_ptr<impl> pimpl;
};

