#pragma once

#include "scriptObject.h"

#include <memory>

/**
 *
 */
class Command: public ScriptObject {
public:
	/**
	 * constructor
	 *
	 * @param name command name
	 * @param nParams number of parameters
	 * @param line line number in script
	 * @param position position in line
	 */
	Command(const std::string & name, int nParams, int line, int position);

	/** no copy constructor */
	Command(const Command &) = delete;

	/** no move constructor */
	Command(Command&&) = delete;

	/** destructor */
	~Command();

	/** no copy */
	Command &operator=(const Command &) = delete;

	/**
	 * get command name
	 *
	 * @return command name
	 */
	const std::string & getName() const;

	/**
	 * get number of parameters command requires
	 *
	 * @return number of parameters
	 */
	int getNumParameters() const;

	int getLine() const;
	int getPosition() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
