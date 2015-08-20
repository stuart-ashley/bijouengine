#pragma once

#include "scriptObject.h"

#include <memory>
#include <string>
#include <unordered_map>

class Caller {
public:
	/**
	 * constructor
	 *
	 * @param filename script filename
	 * @param line line in script
	 * @param locals locals snapshot
	 */
	Caller(const std::string & filename, int line,
			const std::unordered_map<std::string, ScriptObjectPtr> & locals);

	/**
	 * destructor
	 */
	~Caller();

	/**
	 * get filename of script
	 *
	 * @return name of file
	 */
	const std::string & getFilename() const;

	/**
	 * get current line in script
	 *
	 * @return line number
	 */
	int getLine() const;

	/**
	 * get local variables
	 *
	 * @return local variables
	 */
	const std::unordered_map<std::string, ScriptObjectPtr> & getLocals() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

