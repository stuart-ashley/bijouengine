#pragma once

#include "scriptObject.h"

#include <memory>
#include <string>
#include <unordered_map>

class ExceptionHandler;
class BreakpointHandler;
class ScriptException;

class ScriptExecutionState {
public:
	ScriptExecutionState();
	~ScriptExecutionState();

	void pushState(const std::string & filename, int line,
			const std::unordered_map<std::string, ScriptObjectPtr> & locals);

	void popState();

	void setExceptionHandler(const std::shared_ptr<ExceptionHandler> & handler);

	bool hasExceptionHandler() const;

	void handleException(const ScriptException & e);

	void setBreakpointHandler(
			const std::shared_ptr<BreakpointHandler> & handler);

	bool hasBreakpointHandler() const;

	bool handleBreakpoint();

	bool inStepMode() const;

	void setStepMode(bool mode);

	bool exit() const;

	void setExit(bool exit);
private:
	struct impl;
	std::unique_ptr<impl> pimpl;

};

