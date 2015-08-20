#include "procedure.h"

#include "bool.h"
#include "branch.h"
#include "breakpointMarker.h"
#include "command.h"
#include "executable.h"
#include "function.h"
#include "functor.h"
#include "parameters.h"
#include "placeholder.h"
#include "program.h"
#include "real.h"
#include "scriptClass.h"
#include "scriptException.h"
#include "scriptExecutionException.h"
#include "scriptExecutionState.h"
#include "scriptTerminationException.h"
#include "string.h"

#include <cassert>
#include <iostream>
#include <stack>

struct Procedure::impl {
	/** parent program */
	std::shared_ptr<Program> parent;
	/** line in parent */
	int line;
	/** position in line */
	int pos;
	/** parameters */
	std::shared_ptr<Parameters> parameters;
	/** code */
	std::vector<ScriptObjectPtr> elements;

	/*
	 *
	 */
	impl(const std::shared_ptr<Program> & parent, int line, int pos,
			const std::shared_ptr<Parameters> & parameters,
			const std::vector<ScriptObjectPtr> & elements) :
					parent(parent),
					line(line),
					pos(pos),
					parameters(parameters),
					elements(elements) {
	}

	/*
	 *
	 */
	void error(const std::string & desc, int line, int pos) {
		throw ScriptException(desc, parent->getFilename(), line, pos);
	}

	/*
	 *
	 */
	void newClassInstance(const ScriptObjectPtr & element, int line,
			int nParams, ScriptExecutionState & execState,
			const std::unordered_map<std::string, ScriptObjectPtr> & locals,
			std::stack<ScriptObjectPtr> & stack) {
		auto classObj = std::static_pointer_cast<ScriptClass>(element);

		// new instance
		auto instance = classObj->newInstance();

		// init instance
		auto init = std::static_pointer_cast<Procedure>(
				instance->getMember(execState, "__init__"));

		execState.pushState(parent->getFilename(), line, locals);

		init->execProc(execState, instance, nParams, stack);

		execState.popState();

		stack.emplace(instance);
	}

	/*
	 *
	 */
	void callProcedure(const ScriptObjectPtr & self,
			const ScriptObjectPtr & element, int line, int nParams,
			ScriptExecutionState & execState,
			const std::unordered_map<std::string, ScriptObjectPtr> & locals,
			std::stack<ScriptObjectPtr> & stack) {
		auto proc = std::static_pointer_cast<Procedure>(element);

		execState.pushState(parent->getFilename(), line, locals);

		proc->execProc(execState, self, nParams, stack);

		execState.popState();
	}

	/*
	 *
	 */
	void command(ScriptExecutionState & execState, const Command & command,
			const std::unordered_map<std::string, ScriptObjectPtr> & locals,
			std::stack<ScriptObjectPtr> & stack) {
		const auto & commandName = command.getName();
		const auto nParams = command.getNumParameters();
		const auto line = command.getLine();
		const auto position = command.getPosition();

		auto target = stack.top();
		stack.pop();

		if (commandName == "getMember") {
			assert(typeid(*stack.top()) == typeid(String));

			auto name =
					std::static_pointer_cast<String>(stack.top())->getValue();
			stack.pop();

			try {
				const auto & member = target->getMember(execState, name);
				if (typeid(*member) == typeid(Procedure)) {
					auto procedure = std::static_pointer_cast<Procedure>(member);
					stack.emplace(std::make_shared<Functor>(target, procedure));
				} else {
					stack.emplace(member);
				}
			} catch (ScriptExecutionException & e) {
				error(e.what(), line, position);
			}

			return;
		}

		if (commandName == "setMember") {
			assert(typeid(*stack.top()) == typeid(String));

			auto name =
					std::static_pointer_cast<String>(stack.top())->getValue();
			stack.pop();

			try {
				target->setMember(name, stack.top());
				stack.pop();
			} catch (ScriptExecutionException & e) {
				error(e.what(), line, position);
			}
			return;
		}

		ScriptObjectPtr func;
		try {
			func = target->getMember(execState, commandName);
		} catch (ScriptExecutionException & e) {
			error(e.what(), line, position);
		}

		if (typeid(*func) == typeid(ScriptClass)) {
			newClassInstance(func, line, nParams, execState, locals, stack);
		} else if (typeid(*func) == typeid(Procedure)) {
			callProcedure(target, func, line, nParams, execState, locals,
					stack);
		} else {
			auto exec = std::dynamic_pointer_cast<Executable>(func);
			if (exec != nullptr) {
				exec->execute(target, nParams, stack);
				return;
			}
		}
	}

	/*
	 *
	 */
	void function(ScriptExecutionState & execState, const Function & function,
			std::unordered_map<std::string, ScriptObjectPtr> & locals,
			std::stack<ScriptObjectPtr> & stack) {
		const auto & funcName = function.getName();
		const auto nParams = function.getNumParameters();
		const auto line = function.getLine();
		const auto position = function.getPosition();

		if (funcName == "set") {
			assert(nParams == 2);

			auto p = stack.top();
			stack.pop();

			assert(typeid(*p) == typeid(String));

			auto name = std::static_pointer_cast<String>(p)->getValue();

			auto value = stack.top();
			stack.pop();

			if (locals.find(name) != locals.end()) {
				locals[name] = value;
			} else if (parent->hasMember(name)) {
				parent->setMember(name, value);
			} else {
				locals[name] = value;
			}
			return;
		}

		ScriptObjectPtr e;

		auto entry = locals.find(funcName);
		if (entry != locals.end()) {
			e = entry->second;
		} else {
			try {
				e = parent->getMember(execState, funcName);
			} catch (ScriptExecutionException & e) {
				error("Unknown function '" + funcName + "'", line, position);
			}
		}

		try {
			if (typeid(*e) == typeid(ScriptClass)) {
				newClassInstance(e, line, nParams, execState, locals, stack);
				return;
			}

			if (typeid(*e) == typeid(Procedure)) {
				callProcedure(nullptr, e, line, nParams, execState, locals,
						stack);
				return;
			}

			auto exec = std::dynamic_pointer_cast<Executable>(e);
			if (exec != nullptr) {
				exec->execute(nullptr, nParams, stack);
				return;
			}
		} catch (ScriptExecutionException & exception) {
			error(exception.what(), line, position);
		}

		error("Unknown function '" + funcName + "'", line, position);
	}

	/*
	 *
	 */
	void exec(ScriptExecutionState & execState,
			std::unordered_map<std::string, ScriptObjectPtr> & locals,
			std::stack<ScriptObjectPtr> & stack) {
		std::stack<int> exceptions;
		for (size_t counter = 0, n = elements.size(); counter < n; ++counter) {
			try {
				const auto & obj = elements[counter];
				if (typeid(*obj) == typeid(BreakpointMarker)) {
					if (execState.hasBreakpointHandler()) {
						auto bm = std::static_pointer_cast<BreakpointMarker>(
								obj);
						if (execState.exit()) {
							execState.setExit(false);
							throw ScriptTerminationException();
						}
						if (execState.inStepMode()
								|| (bm->isActive() && bm->isEnabled())) {
							execState.pushState(parent->getFilename(),
									bm->getStartLine(), locals);
							if (execState.handleBreakpoint() == false) {
								throw ScriptTerminationException();
							}
							execState.popState();
						}
					}
					continue;
				}
				/* branch ScriptObjectPtr */
				if (typeid(*obj) == typeid(Branch)) {
					auto type =
							std::static_pointer_cast<Branch>(obj)->getType();
					/* branch offset, minus loop increment */
					auto offset =
							std::static_pointer_cast<Branch>(obj)->getOffset()
									- 1;

					/* branch */
					if (type == Branch::Type::B) {
						counter += offset;
						continue;
					}

					/* branch if */
					if (type == Branch::Type::BIF) {
						if (stack.top() == Bool::True()) {
							counter += offset;
						}
						stack.pop();
						continue;
					}

					/* branch not if */
					if (type == Branch::Type::BNIF) {
						if (stack.top() == Bool::False()) {
							counter += offset;
						}
						stack.pop();
						continue;
					}
				}
				if (typeid(*obj) == typeid(Command)) {
					auto cmd = std::static_pointer_cast<Command>(obj);
					command(execState, *cmd, locals, stack);
					continue;
				}
				if (typeid(*obj) == typeid(Function)) {
					auto fn = std::static_pointer_cast<Function>(obj);
					if (fn->getName() == "pushExceptionHandler") {
						// always an integer
						assert(
								typeid(*stack.top()) == typeid(Real)
										&& std::static_pointer_cast<Real>(
												stack.top())->isInt32());
						int offset =
								std::static_pointer_cast<Real>(stack.top())->getInt32();
						stack.pop();
						exceptions.emplace(counter + offset);
						continue;
					}
					if (fn->getName() == "popExceptionHandler") {
						exceptions.pop();
						continue;
					}
					function(execState, *fn, locals, stack);
					continue;
				}

				if (typeid(*obj) == typeid(Placeholder)) {
					auto placeholder = std::static_pointer_cast<Placeholder>(
							obj);
					const auto & name = placeholder->getName();

					auto entry = locals.find(name);
					if (entry != locals.end()) {
						stack.emplace(entry->second);
					} else {
						try {
							stack.emplace(parent->getMember(execState, name));
						} catch (ScriptExecutionException & e) {
							error("Unknown variable '" + name + "'",
									placeholder->getLine(),
									placeholder->getPosition());
						}
					}
					continue;
				}

				// default
				stack.emplace(obj);
			} catch (ScriptException & e) {
				if (exceptions.empty()) {
					if (execState.hasExceptionHandler()) {
						execState.pushState(parent->getFilename(), e.getLine(),
								locals);
						execState.handleException(e);
						throw ScriptTerminationException();
					} else {
						throw;
					}
				}
				/* minus loop increment */
				counter = exceptions.top() - 1;
				exceptions.pop();

				stack.emplace(std::make_shared<String>(e.toString()));
			}
		}
	}
};

/*
 *
 */
Procedure::Procedure(const std::shared_ptr<Program> & parent, int line, int pos,
		const std::shared_ptr<Parameters> & parameters,
		const std::vector<ScriptObjectPtr> & elements) :
		pimpl(new impl(parent, line, pos, parameters, elements)) {

}

/*
 *
 */
Procedure::~Procedure() {
}

/*
 *
 */
void Procedure::execProc(ScriptExecutionState & execState,
		const ScriptObjectPtr & self, int nArgs,
		std::stack<ScriptObjectPtr> & stack) {
	std::unordered_map<std::string, ScriptObjectPtr> locals;
	if (pimpl->parameters != nullptr) {
		try {
			locals = pimpl->parameters->getArgs(nArgs, stack);
		} catch (ScriptExecutionException & e) {
			throw ScriptException(e.what(), pimpl->parent->getFilename(),
					pimpl->line, pimpl->pos);
		}
	}

	if (self != nullptr) {
		locals["this"] = self;
	}
	pimpl->exec(execState, locals, stack);
}

/*
 *
 */
std::vector<std::shared_ptr<BreakpointMarker> > Procedure::getBreakpoints() const {
	std::vector<std::shared_ptr<BreakpointMarker> > breakpoints;
	for (const auto & e : pimpl->elements) {
		if (typeid(*e) == typeid(BreakpointMarker)) {
			auto bp = std::static_pointer_cast<BreakpointMarker>(e);
			breakpoints.emplace_back(bp);
		}
	}
	return breakpoints;
}

/**
 *
 * @param instance
 */
void Procedure::patchInstance(const ScriptObjectPtr & instance) {
	for (auto & element : pimpl->elements) {
		if (element == nullptr) {
			element = instance;
		}
	}
}
