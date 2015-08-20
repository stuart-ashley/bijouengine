#pragma once

#include "scriptObject.h"

#include <vector>
#include <stack>

class Parameters;
class BreakpointMarker;
class Program;

class Procedure: public ScriptObject {
public:

	Procedure(const std::shared_ptr<Program> & parent, int line, int pos,
			const std::shared_ptr<Parameters> & parameters,
			const std::vector<ScriptObjectPtr> & elements);

	~Procedure();

	void execProc(ScriptExecutionState & execState,
			const ScriptObjectPtr & self, int nArgs,
			std::stack<ScriptObjectPtr> & stack);

	std::vector<std::shared_ptr<BreakpointMarker> > getBreakpoints() const;

	/**
	 *
	 * @param instance
	 */
	void patchInstance(const ScriptObjectPtr & instance);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

