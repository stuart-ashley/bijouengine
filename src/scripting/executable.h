#pragma once

#include <memory>
#include <stack>
#include "scriptObject.h"

class Executable: public ScriptObject {
public:
	virtual ~Executable() {
	}

	virtual void execute(const std::shared_ptr<ScriptObject> & self,
			unsigned int nArgs,
			std::stack<std::shared_ptr<ScriptObject> > & stack) const = 0;
};
