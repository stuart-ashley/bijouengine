#pragma once

#include "scriptObject.h"

#include <memory>

class Function: public ScriptObject {
public:
	Function(const std::string & name, int nParams, int line, int position);
	~Function();

	const std::string & getName() const;
	int getNumParameters() const;
	int getLine() const;
	int getPosition() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
