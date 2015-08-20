#pragma once

#include "procedure.h"

class Functor final: public ScriptObject {
public:

	inline Functor(const ScriptObjectPtr & instance,
			const std::shared_ptr<Procedure> & procedure) :
			m_instance(instance), m_procedure(procedure) {
	}

	inline virtual ~Functor() = default;

	inline void exec(ScriptExecutionState & execState, int nArgs,
			std::stack<ScriptObjectPtr> & stack) {
		m_procedure->execProc(execState, m_instance, nArgs, stack);
	}
private:
	ScriptObjectPtr m_instance;
	std::shared_ptr<Procedure> m_procedure;
};

