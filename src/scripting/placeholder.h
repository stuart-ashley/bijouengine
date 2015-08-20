#pragma once

#include "scriptObject.h"

#include <memory>

class Placeholder: public ScriptObject {
public:
	Placeholder(const std::string & name, int line, int position);
	~Placeholder();

	const std::string & getName() const;
	int getLine() const;
	int getPosition() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
