#include "scriptExecutionState.h"

#include "breakpointHandler.h"
#include "caller.h"
#include "exceptionHandler.h"

#include <atomic>

struct ScriptExecutionState::impl {
	std::stack<std::shared_ptr<Caller>> callstack;

	std::atomic<bool> stepMode;
	std::atomic<bool> exit;

	std::shared_ptr<ExceptionHandler> exceptionHandler = nullptr;
	std::shared_ptr<BreakpointHandler> breakpointHandler = nullptr;

	impl() :
			stepMode(false), exit(false) {
	}
};

/*
 *
 */
ScriptExecutionState::ScriptExecutionState() :
		pimpl(new impl()) {
}

/*
 *
 */
ScriptExecutionState::~ScriptExecutionState()  {
}

void ScriptExecutionState::pushState(const std::string & filename, int line,
		const std::unordered_map<std::string, ScriptObjectPtr> & locals) {
	pimpl->callstack.emplace(std::make_shared<Caller>(filename, line, locals));
}

void ScriptExecutionState::popState() {
	pimpl->callstack.pop();
}

void ScriptExecutionState::setExceptionHandler(
		const std::shared_ptr<ExceptionHandler> & handler) {
	pimpl->exceptionHandler = handler;
}

bool ScriptExecutionState::hasExceptionHandler() const {
	return pimpl->exceptionHandler != nullptr;
}

void ScriptExecutionState::handleException(const ScriptException & e) {
	pimpl->exceptionHandler->handle(e, pimpl->callstack);
}

void ScriptExecutionState::setBreakpointHandler(
		const std::shared_ptr<BreakpointHandler> & handler) {
	pimpl->breakpointHandler = handler;
}

bool ScriptExecutionState::hasBreakpointHandler() const {
	return pimpl->breakpointHandler != nullptr;
}

bool ScriptExecutionState::handleBreakpoint() {
	return pimpl->breakpointHandler->handle(pimpl->callstack);
}

bool ScriptExecutionState::inStepMode() const {
	return pimpl->stepMode;
}

void ScriptExecutionState::setStepMode(bool mode) {
	pimpl->stepMode = mode;
}

bool ScriptExecutionState::exit() const {
	return pimpl->exit;
}

void ScriptExecutionState::setExit(bool exit) {
	pimpl->exit = exit;
}
