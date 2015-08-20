#pragma once

#include "scriptException.h"

#include <memory>
#include <stack>

class Caller;

class ExceptionHandler {
public:
	virtual ~ExceptionHandler();

	virtual void handle(const ScriptException & e,
			const std::stack<std::shared_ptr<Caller> > & callStack) = 0;
};

