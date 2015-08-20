#pragma once

#include <memory>
#include <stack>

class Caller;

class BreakpointHandler {
public:
	virtual ~BreakpointHandler();

	virtual bool handle(
			const std::stack<std::shared_ptr<Caller> > & callStack) = 0;
};

