#include "program.h"

#include "breakpointMarker.h"
#include "executable.h"
#include "function.h"
#include "list.h"
#include "map.h"
#include "mathModule.h"
#include "none.h"
#include "pair.h"
#include "parser.h"
#include "procedure.h"
#include "scriptClass.h"
#include "scriptException.h"
#include "scriptExecutionState.h"
#include "scriptExecutionException.h"
#include "set.h"
#include "string.h"
#include "tokens.h"

#include <iostream>

namespace {
	auto math = std::make_shared<MathModule>();

	class Print: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			for (unsigned i = 0; i < nArgs; ++i) {
				auto p = stack.top();
				stack.pop();
				std::cout << p->toString();
			}
			std::cout << std::endl;
		}
	};

	auto print = std::make_shared<Print>();
}

struct Program::impl {
	/** initialization code */
	std::shared_ptr<Procedure> m_initProc;
	/** filename */
	std::string m_filename;
	std::shared_ptr<ScriptException> m_error;
	bool m_initialized;
	std::vector<std::shared_ptr<BreakpointMarker> > m_breakpoints;
	std::vector<ScriptObjectPtr> m_usedModules;
	std::unordered_map<std::string, ScriptObjectPtr> m_members;
	std::string m_content;

	impl() :
			m_initialized(false) {
	}

	/*
	 *
	 */
	void parseFunc(const std::shared_ptr<Program> & program,
			Tokens::Iterator & titr) {
		// function name
		auto name = titr.get().getValue();

		// parse def
		auto func = def(titr, program);

		// add function to program members
		m_members[name] = func;

		// add function breakpoints
		auto brks = func->getBreakpoints();
		m_breakpoints.insert(m_breakpoints.end(), brks.begin(), brks.end());
	}

	/*
	 *
	 */
	void parseAssign(Tokens::Iterator & titr,
			std::vector<ScriptObjectPtr> & staticScriptObjectPtrs) {
		// variable name
		auto name = titr.get().getValue();

		// expect identifier
		if (titr.accept(Token::Type::IDENT) == false) {
			titr.error("Require: identifier");
		}

		// expect assignment
		if (titr.accept(Token::Type::ASSIGN) == false) {
			titr.error("Require: '=='");
		}

		// parse expression
		auto expr = expression(titr);
		staticScriptObjectPtrs.insert(staticScriptObjectPtrs.end(),
				expr.begin(), expr.end());

		// get position
		auto line = titr.get().getLine();
		auto pos = titr.get().getPosition();

		// expect semicolon
		if (titr.accept(Token::Type::SEMI) == false) {
			titr.error("Require: ';'");
		}

		// name
		staticScriptObjectPtrs.emplace_back(std::make_shared<String>(name));

		// set
		staticScriptObjectPtrs.emplace_back(
				std::make_shared<Function>("set", 2, line, pos));

		// add variable to program members
		m_members[name] = None::none();
	}

	/*
	 *
	 */
	void parse(const std::shared_ptr<Program> & program,
			std::istream & stream) {
		std::vector<ScriptObjectPtr> staticScriptObjectPtrs;

		char buffer[4096];
		while (stream.read(buffer, sizeof(buffer))) {
			m_content.append(buffer, sizeof(buffer));
		}
		m_content.append(buffer, stream.gcount());

		Tokens tokens(m_content);
		Tokens::Iterator titr(tokens);

		while (titr.hasNext()) {
			if (titr.accept(Token::Type::STATIC_TYPE, Token::Type::DEF)
					|| titr.accept(Token::Type::DEF)) {
				parseFunc(program, titr);
			} else if (titr.accept(Token::Type::STATIC_TYPE)) {
				parseAssign(titr, staticScriptObjectPtrs);
			} else if (titr.accept(Token::Type::CLASS)) {
				// parse class
				auto c = classInstance(titr, program);

				// add class to program members
				m_members[c->getClassName()] = c;

			} else {
				titr.error("Require: 'static', 'class' or 'def'");
			}
		}

		m_initProc = std::make_shared<Procedure>(program, 0, 0, nullptr,
				staticScriptObjectPtrs);
	}
};

/**
 * constructor
 */
Program::Program() :
		pimpl(new impl()) {

}

/**
 * destructor
 */
Program::~Program() {
}

/**
 *
 * @param line
 * @return
 */
std::shared_ptr<BreakpointMarker> Program::getBreakpoint(int line) {
	if (pimpl->m_error != nullptr) {
		throw pimpl->m_error;
	}
	for (const auto & bp : pimpl->m_breakpoints) {
		if (bp->getStartLine() > line
				|| (line >= bp->getStartLine() && line <= bp->getEndLine())) {
			return bp;
		}
	}
	return nullptr;
}

/**
 * get active breakpoints
 *
 * @return list of active breakpoints
 */
std::vector<std::shared_ptr<BreakpointMarker> > Program::getBreakpoints() {
	std::vector<std::shared_ptr<BreakpointMarker> > bpts;

	if (pimpl->m_error == nullptr) {
		for (const auto & bp : pimpl->m_breakpoints) {
			if (bp->isActive()) {
				bpts.emplace_back(bp);
			}
		}
	}

	return bpts;
}

/**
 *
 * @return
 */
const std::string & Program::getFilename() const {
	return pimpl->m_filename;
}

/**
 *
 * @return
 */
const std::string & Program::getContent() const {
	return pimpl->m_content;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Program::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	for (const auto & module : pimpl->m_usedModules) {
		auto member = module->getMember(execState, name);
		if (member != nullptr) {
			return member;
		}
	}
	auto entry = pimpl->m_members.find(name);
	if (entry != pimpl->m_members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @param name
 * @return
 */
bool Program::hasMember(const std::string & name) const {
	return pimpl->m_members.find(name) != pimpl->m_members.end();
}

/**
 *
 * @param execState
 */
void Program::init(ScriptExecutionState & execState) {
	if (pimpl->m_initialized) {
		return;
	}
	if (pimpl->m_error != nullptr) {
		throw pimpl->m_error;
	}
	for (const auto & entry : pimpl->m_members) {
		if (typeid(*entry.second) == typeid(ScriptClass)) {
			std::static_pointer_cast<ScriptClass>(entry.second)->init(
					execState);
		}
	}
	std::stack<ScriptObjectPtr> stack;
	pimpl->m_initProc->execProc(execState, nullptr, 0, stack);
	pimpl->m_initialized = true;
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
OVERRIDE void Program::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	auto result = pimpl->m_members.emplace(name, value);
	if (result.second == false) {
		result.first->second = value;
	}
}

/**
 *
 * @param module
 */
void Program::use(const ScriptObjectPtr & module) {
	pimpl->m_usedModules.emplace_back(module);
}

/**
 *
 * @param filename
 * @param stream
 * @return
 */
STATIC std::shared_ptr<Program> Program::create(const std::string & filename,
		std::istream & stream) {
	auto program = std::shared_ptr<Program>(new Program());

	program->pimpl->m_filename = filename;

	program->pimpl->m_members["__file__"] = std::make_shared<String>(filename);
	program->pimpl->m_members["List"] = List::getFactory();
	program->pimpl->m_members["Map"] = Map::getFactory();
	program->pimpl->m_members["Math"] = math;
	program->pimpl->m_members["Pair"] = Pair::getFactory();
	program->pimpl->m_members["Set"] = Set::getFactory();
	program->pimpl->m_members["print"] = print;

	try {
		program->pimpl->parse(program, stream);
	} catch (ScriptException & e) {
		program->pimpl->m_error = std::make_shared<ScriptException>(e);
		return program;
	}

	return program;
}
