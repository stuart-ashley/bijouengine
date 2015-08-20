#include "parser.h"

#include "branch.h"
#include "breakpointMarker.h"
#include "bool.h"
#include "command.h"
#include "function.h"
#include "kwarg.h"
#include "none.h"
#include "placeholder.h"
#include "parameter.h"
#include "parameters.h"
#include "procedure.h"
#include "program.h"
#include "real.h"
#include "scriptClass.h"
#include "string.h"

#include <deque>
#include <exception>
#include <iostream>
#include <limits>
#include <vector>

namespace {
	std::deque<ScriptObjectPtr> term(Tokens::Iterator & titr);

	std::string getUid() {
		static int uid = 0;
		return "__uid__" + std::to_string(++uid);
	}

	/**
	 * Append parameters to a list
	 *
	 * @param titr token iterator
	 * @param list existing list
	 * @return number of parameters
	 */
	int parameters(Tokens::Iterator & titr,
			std::deque<ScriptObjectPtr> & list) {
		if (!titr.accept(Token::Type::LPAREN)) {
			titr.error("expecting: '('");
			return 0;
		}
		if (titr.accept(Token::Type::RPAREN)) {
			return 0;
		}

		std::deque<ScriptObjectPtr> params;

		int nParams = 0;
		do {
			std::string name = titr.get().getValue();
			if (titr.accept(Token::Type::IDENT, Token::Type::ASSIGN)) {
				params.insert(params.begin(), std::make_shared<Kwarg>(name));
				auto list2 = expression(titr);
				params.insert(params.begin(), list2.begin(), list2.end());
			} else {
				auto list2 = expression(titr);
				params.insert(params.begin(), list2.begin(), list2.end());
			}
			++nParams;
		} while (titr.accept(Token::Type::COMMA));

		if (!titr.accept(Token::Type::RPAREN)) {
			titr.error("expecting: ')'");
		}

		list.insert(list.end(), params.begin(), params.end());
		return nParams;
	}

	/**
	 * Add members to a list, if any
	 *
	 * @param titr token iterator
	 * @param list existing list
	 */
	void members(Tokens::Iterator & titr, std::deque<ScriptObjectPtr> & list) {
		if (titr.accept(Token::Type::DOT)) {
			// get potential member name
			auto member = titr.get().getValue();
			// expecting identifier for member name
			if (!titr.accept(Token::Type::IDENT)) {
				titr.error("expecting: IDENT");
			}
			// parameters or not
			if (titr.get().isType(Token::Type::LPAREN)) {
				// get parameters
				std::deque<ScriptObjectPtr> params;
				int nParams = parameters(titr, params);
				// insert parameters at front
				list.insert(list.begin(), params.begin(), params.end());
				// insert command at back
				list.emplace_back(
						std::make_shared<Command>(member, nParams,
								titr.get().getLine(),
								titr.get().getPosition()));
			} else {
				// insert member name at front
				list.push_front(std::make_shared<String>(member));
				// insert get command at back
				list.emplace_back(
						std::make_shared<Command>("getMember", 2,
								titr.get().getLine(),
								titr.get().getPosition()));
			}
			// following members
			members(titr, list);
		}
	}

	/*
	 */
	std::deque<ScriptObjectPtr> term(Tokens::Iterator & titr) {
		std::string name = titr.get().getValue();
		if (titr.accept(Token::Type::IDENT)) {
			std::deque<ScriptObjectPtr> list;
			if (titr.get().isType(Token::Type::LPAREN)) {
				int nParams = parameters(titr, list);
				list.emplace_back(
						std::make_shared<Function>(name, nParams,
								titr.get().getLine(),
								titr.get().getPosition()));
			} else {
				list.emplace_back(
						std::make_shared<Placeholder>(name,
								titr.get().getLine(),
								titr.get().getPosition()));
			}
			members(titr, list);
			return list;
		}

		if (titr.accept(Token::Type::LPAREN)) {
			std::deque<ScriptObjectPtr> list = expression(titr);
			if (titr.accept(Token::Type::RPAREN) == false) {
				titr.error("expect: ')'");
			}
			members(titr, list);
			return list;
		}

		auto c = constTerm(titr);
		if (c != nullptr) {
			std::deque<ScriptObjectPtr> list;
			list.emplace_back(c);
			return list;
		}

		titr.error("term: syntax error");
		return std::deque<ScriptObjectPtr>();
	}

	/*
	 */
	std::deque<ScriptObjectPtr> assignment(Tokens::Iterator & titr) {
		std::string identifier = titr.get().getValue();
		if (titr.accept(Token::Type::IDENT, Token::Type::ASSIGN)) {
			auto list = term(titr);
			list.emplace_back(std::make_shared<String>(identifier));
			list.emplace_back(
					std::make_shared<Function>("set", 2, titr.get().getLine(),
							titr.get().getPosition()));
			if (!titr.accept(Token::Type::SEMI)) {
				titr.error("expect: ';'");
			}
			return list;
		}
		titr.error("assignment: syntax error");
		return std::deque<ScriptObjectPtr>();
	}

	/*
	 *
	 */
	std::vector<ScriptObjectPtr> block(Tokens::Iterator & titr) {
		std::vector<ScriptObjectPtr> list;
		if (titr.accept(Token::Type::BEGIN) == false) {
			titr.error("expect: '{'");
		}
		while (titr.accept(Token::Type::END) == false) {
			auto stmt = statement(titr);
			list.insert(list.end(), stmt.begin(), stmt.end());
		}
		return list;
	}
}

/*
 */
ScriptObjectPtr constTerm(Tokens::Iterator & titr) {
	std::string value = titr.get().getValue();
	if (titr.accept(Token::Type::TRUE)) {
		return Bool::True();
	}
	if (titr.accept(Token::Type::FALSE)) {
		return Bool::False();
	}
	if (titr.accept(Token::Type::NONE)) {
		return None::none();
	}
	if (titr.accept(Token::Type::STRING)) {
		return std::make_shared<String>(value);
	}
	if (titr.accept(Token::Type::NUMBER)) {
		return std::make_shared<Real>(std::stod(value));
	}
	titr.error("expecting const term");
	return nullptr;
}

/*
 */
std::deque<ScriptObjectPtr> expression(Tokens::Iterator & titr) {
	if (titr.get().getValue() == "-" && titr.accept(Token::Type::OP)) {
		std::deque<ScriptObjectPtr> list = term(titr);
		list.emplace_back(
				std::make_shared<Command>("__neg__", 0, titr.get().getLine(),
						titr.get().getPosition()));
		return list;
	}
	std::deque<ScriptObjectPtr> lhs = term(titr); /* lhs */
	if (titr.accept(Token::Type::COLON)) {
		std::deque<ScriptObjectPtr> list = term(titr); /* rhs */
		list.insert(list.end(), lhs.begin(), lhs.end());
		list.emplace_back(
				std::make_shared<Function>("Pair", 2, titr.get().getLine(),
						titr.get().getPosition()));
		return list;
	}
	auto op = titr.get();
	if (titr.accept(Token::Type::OP)) {
		std::deque<ScriptObjectPtr> list = term(titr); /* rhs */
		list.insert(list.end(), lhs.begin(), lhs.end());
		auto value = op.getValue();
		auto line = op.getLine();
		auto pos = op.getPosition();
		if (value == "+") {
			list.emplace_back(
					std::make_shared<Command>("__add__", 1, line, pos));
		} else if (value == "-") {
			list.emplace_back(
					std::make_shared<Command>("__sub__", 1, line, pos));
		} else if (value == "*") {
			list.emplace_back(
					std::make_shared<Command>("__mul__", 1, line, pos));
		} else if (value == "/") {
			list.emplace_back(
					std::make_shared<Command>("__div__", 1, line, pos));
		} else if (value == "^") {
			list.emplace_back(
					std::make_shared<Command>("__pow__", 1, line, pos));
		} else if (value == ">") {
			list.emplace_back(
					std::make_shared<Command>("__gt__", 1, line, pos));
		} else if (value == "<") {
			list.emplace_back(
					std::make_shared<Command>("__lt__", 1, line, pos));
		} else if (value == ">=") {
			list.emplace_back(
					std::make_shared<Command>("__gte__", 1, line, pos));
		} else if (value == "<=") {
			list.emplace_back(
					std::make_shared<Command>("__lte__", 1, line, pos));
		} else if (value == "==") {
			list.emplace_back(
					std::make_shared<Command>("__eq__", 1, line, pos));
		} else if (value == "!=") {
			list.emplace_back(
					std::make_shared<Command>("__neq__", 1, line, pos));
		} else if (value == "&&") {
			list.emplace_back(
					std::make_shared<Command>("__and__", 1, line, pos));
		} else if (value == "||") {
			list.emplace_back(
					std::make_shared<Command>("__or__", 1, line, pos));
		} else {
			titr.error("Expression syntax error, unknown op '" + value + "'");
		}
		return list;
	} else {
		return lhs;
	}
}

/*
 *
 */
std::deque<ScriptObjectPtr> statement(Tokens::Iterator & titr) {
	auto s = titr.get();
	auto line = s.getLine();
	if (titr.accept(Token::Type::IDENT)) {
		if (titr.accept(Token::Type::ASSIGN)) {
			auto list = expression(titr);
			list.emplace_back(std::make_shared<String>(s.getValue()));
			list.emplace_back(
					std::make_shared<Function>("set", 2, titr.get().getLine(),
							titr.get().getPosition()));
			list.push_front(
					std::make_shared<BreakpointMarker>(line,
							titr.get().getLine()));
			if (titr.accept(Token::Type::SEMI) == false) {
				titr.error("expecting: ';'");
			}
			return list;
		}
		std::deque<ScriptObjectPtr> list;
		if (titr.get().getType() == Token::Type::LPAREN) {
			auto nParams = parameters(titr, list);
			list.emplace_back(
					std::make_shared<Function>(s.getValue(), nParams,
							titr.get().getLine(), titr.get().getPosition()));
		} else {
			list.emplace_back(
					std::make_shared<Placeholder>(s.getValue(),
							titr.get().getLine(), titr.get().getPosition()));
		}
		members(titr, list);

		auto e = list.back();

		if (typeid(*e) == typeid(Command)
				&& std::static_pointer_cast<Command>(e)->getName()
						== "getMember") {
			if (titr.accept(Token::Type::ASSIGN) == false) {
				titr.error("expecting: '='");
			}
			auto expr = expression(titr);
			list.insert(list.begin(), expr.begin(), expr.end());
			list[list.size() - 1] = std::make_shared<Command>("setMember", 3,
					titr.get().getLine(), titr.get().getPosition());
		}

		list.push_front(
				std::make_shared<BreakpointMarker>(line, titr.get().getLine()));
		if (titr.accept(Token::Type::SEMI) == false) {
			titr.error("expecting: ';'");
		}
		return list;
	}
	if (titr.accept(Token::Type::IF)) {
		std::vector<size_t> endBranchIndices;
		std::deque<ScriptObjectPtr> list;
		/* condition */
		if (titr.accept(Token::Type::LPAREN) == false) {
			titr.error("expect: '('");
		}
		auto expr = expression(titr);
		list.insert(list.begin(), expr.begin(), expr.end());
		if (titr.accept(Token::Type::RPAREN) == false) {
			titr.error("expect: ')'");
		}
		/* not if, branch to next condition */
		auto nextBranchIndex = list.size();
		list.emplace_back(nullptr);
		/* block */
		auto blk = block(titr);
		list.insert(list.end(), blk.begin(), blk.end());
		/* branch to end */
		endBranchIndices.emplace_back(list.size());
		list.emplace_back(nullptr);
		/* for each elif */
		while (titr.accept(Token::Type::ELIF)) {
			/* patch next branch */
			list[nextBranchIndex] = std::make_shared<Branch>(Branch::Type::BNIF,
					list.size() - nextBranchIndex);
			/* condition */
			if (titr.accept(Token::Type::LPAREN) == false) {
				titr.error("expect: '('");
			}
			auto expr2 = expression(titr);
			list.insert(list.end(), expr2.begin(), expr2.end());
			if (titr.accept(Token::Type::RPAREN) == false) {
				titr.error("expect: ')'");
			}
			/* not elif, branch to next condition */
			nextBranchIndex = list.size();
			list.emplace_back(nullptr);
			/* block */
			auto blk2 = block(titr);
			list.insert(list.end(), blk2.begin(), blk2.end());
			/* branch to end */
			endBranchIndices.emplace_back(list.size());
			list.emplace_back(nullptr);
		}
		/* patch next branch */
		list[nextBranchIndex] = std::make_shared<Branch>(Branch::Type::BNIF,
				list.size() - nextBranchIndex);
		/* else */
		if (titr.accept(Token::Type::ELSE)) {
			auto blk2 = block(titr);
			list.insert(list.end(), blk2.begin(), blk2.end());
		}
		/* patch end branches */
		auto endIndex = list.size();
		for (auto i : endBranchIndices) {
			list[i] = std::make_shared<Branch>(Branch::Type::B, endIndex - i);
		}
		return list;
	}
	if (titr.accept(Token::Type::FOR)) {
		std::deque<ScriptObjectPtr> fragment;
		/* temporary variable */
		auto list = getUid();
		auto count = getUid();
		auto itr = getUid();

		/* open parenthesis */
		if (titr.accept(Token::Type::LPAREN) == false) {
			titr.error("expect: '('");
		}
		/* for each var */
		auto var = titr.get().getValue();
		if (titr.accept(Token::Type::IDENT) == false) {
			titr.error("expect: identifier");
		}
		/* colon */
		if (titr.accept(Token::Type::COLON) == false) {
			titr.error("expect: ':'");
		}
		/* assign expression to list */
		auto expr = expression(titr);
		fragment.insert(fragment.end(), expr.begin(), expr.end());
		fragment.emplace_back(std::make_shared<String>(list));
		fragment.emplace_back(
				std::make_shared<Function>("set", 2, titr.get().getLine(),
						titr.get().getPosition()));
		/* assign count list size */
		fragment.emplace_back(
				std::make_shared<Placeholder>(list, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Command>("size", 0, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(std::make_shared<String>(count));
		fragment.emplace_back(
				std::make_shared<Function>("set", 2, titr.get().getLine(),
						titr.get().getPosition()));
		/* check count > 0 */
		fragment.emplace_back(std::make_shared<Real>(0));
		fragment.emplace_back(
				std::make_shared<Placeholder>(count, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Command>("__gt__", 1, titr.get().getLine(),
						titr.get().getPosition()));
		auto endBranchIndex = fragment.size();
		fragment.emplace_back(nullptr);
		/* initialise itr */
		fragment.emplace_back(std::make_shared<Real>(0));
		fragment.emplace_back(std::make_shared<String>(itr));
		fragment.emplace_back(
				std::make_shared<Function>("set", 2, titr.get().getLine(),
						titr.get().getPosition()));
		/* loop label */
		auto loopIndex = fragment.size();
		/* close parenthesis */
		if (titr.accept(Token::Type::RPAREN) == false) {
			titr.error("expect: ')'");
		}
		/* set var */
		fragment.emplace_back(
				std::make_shared<Placeholder>(itr, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Placeholder>(list, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Command>("get", 1, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(std::make_shared<String>(var));
		fragment.emplace_back(
				std::make_shared<Function>("set", 2, titr.get().getLine(),
						titr.get().getPosition()));
		/* block */
		auto blk = block(titr);
		fragment.insert(fragment.end(), blk.begin(), blk.end());
		/* increment itr */
		fragment.emplace_back(std::make_shared<Real>(1));
		fragment.emplace_back(
				std::make_shared<Placeholder>(itr, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Command>("__add__", 1, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(std::make_shared<String>(itr));
		fragment.emplace_back(
				std::make_shared<Function>("set", 2, titr.get().getLine(),
						titr.get().getPosition()));
		/* while itr < count */
		fragment.emplace_back(
				std::make_shared<Placeholder>(count, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Placeholder>(itr, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Command>("__lt__", 1, titr.get().getLine(),
						titr.get().getPosition()));
		fragment.emplace_back(
				std::make_shared<Branch>(Branch::Type::BIF,
						loopIndex - fragment.size()));
		/* patch end branch */
		fragment[endBranchIndex] = std::make_shared<Branch>(Branch::Type::BNIF,
				fragment.size() - endBranchIndex);
		/* done */
		return fragment;
	}
	if (titr.accept(Token::Type::WHILE)) {
		std::deque<ScriptObjectPtr> fragment;

		/* open parenthesis */
		if (titr.accept(Token::Type::LPAREN) == false) {
			titr.error("expect: '('");
		}
		/* condition */
		auto expr = expression(titr);
		fragment.insert(fragment.end(), expr.begin(), expr.end());
		/* end branch */
		auto endBranchIndex = fragment.size();
		fragment.emplace_back(nullptr);
		/* close parenthesis */
		if (titr.accept(Token::Type::RPAREN) == false) {
			titr.error("expect: ')'");
		}
		/* block */
		auto blk = block(titr);
		fragment.insert(fragment.end(), blk.begin(), blk.end());
		/* loop to beginning */
		fragment.emplace_back(
				std::make_shared<Branch>(Branch::Type::B, -static_cast<int>(fragment.size())));
		/* patch end branch */
		fragment[endBranchIndex] = std::make_shared<Branch>(Branch::Type::BNIF,
				fragment.size() - endBranchIndex);
		/* done */
		return fragment;
	}
	if (titr.accept(Token::Type::RETURN)) {
		std::deque<ScriptObjectPtr> list;
		if (titr.get().getType() != Token::Type::SEMI) {
			auto expr = expression(titr);
			list.insert(list.end(), expr.begin(), expr.end());
		}
		/* jump far beyond end without overflow */
		list.emplace_back(
				std::make_shared<Branch>(Branch::Type::B,
						std::numeric_limits<int>::max() / 2));
		if (titr.accept(Token::Type::SEMI) == false) {
			titr.error("expect: ';'");
		}
		return list;
	}
	if (titr.accept(Token::Type::TRY)) {
		std::deque<ScriptObjectPtr> list;
		/* push exception handler */
		auto catchBranchIndex = list.size();
		list.emplace_back(nullptr);
		list.emplace_back(
				std::make_shared<Function>("pushExceptionHandler", 1,
						titr.get().getLine(), titr.get().getPosition()));

		/* block */
		auto blk = block(titr);
		list.insert(list.end(), blk.begin(), blk.end());

		/* pop exception handler */
		list.emplace_back(
				std::make_shared<Function>("popExceptionHandler", 0,
						titr.get().getLine(), titr.get().getPosition()));

		/* branch to end */
		auto endBranchIndex = list.size();
		list.emplace_back(nullptr);

		/* catch */
		if (titr.accept(Token::Type::CATCH) == false) {
			titr.error("expecting: 'catch'");
		}
		list[catchBranchIndex] = std::make_shared<Real>(
				list.size() - catchBranchIndex - 1.);

		/* var */
		if (titr.accept(Token::Type::COLON)) {
			auto var = titr.get().getValue();
			if (titr.accept(Token::Type::IDENT) == false) {
				titr.error("expect: identifier");
			}
			list.emplace_back(std::make_shared<String>(var));
			list.emplace_back(
					std::make_shared<Function>("set", 2, titr.get().getLine(),
							titr.get().getPosition()));
		}

		/* block */
		auto blk2 = block(titr);
		list.insert(list.end(), blk2.begin(), blk2.end());

		/* patch end branch */
		list[endBranchIndex] = std::make_shared<Branch>(Branch::Type::B,
				list.size() - endBranchIndex);
		return list;
	}
	titr.error("Statement syntax error");
	return std::deque<ScriptObjectPtr>();
}

std::shared_ptr<Procedure> def(Tokens::Iterator & titr,
		const std::shared_ptr<Program> & parent) {
	auto line = titr.get().getLine();
	auto pos = titr.get().getPosition();
	if (titr.accept(Token::Type::IDENT) == false) {
		titr.error("expect: identifier");
	}
	if (titr.accept(Token::Type::LPAREN) == false) {
		titr.error("expect: '('");
	}
	std::vector<BaseParameter> params;
	if (titr.get().getType() != Token::Type::RPAREN) {
		do {
			auto name = titr.get().getValue();
			if (titr.accept(Token::Type::IDENT) == false) {
				titr.error("expect: identifier");
			}
			if (titr.accept(Token::Type::ASSIGN)) {
				auto term = constTerm(titr);
				if (term == nullptr) {
					titr.error("Expect constant term for '" + name + "'");
				}
				params.emplace_back(name, nullptr, term);
			} else {
				params.emplace_back(name, nullptr, nullptr);
			}
		} while (titr.accept(Token::Type::COMMA));
	}

	if (titr.accept(Token::Type::RPAREN) == false) {
		titr.error("expect: ')'");
	}
	auto blk = block(titr);
	return std::make_shared<Procedure>(parent, line, pos,
			std::make_shared<Parameters>(params), blk);
}

std::shared_ptr<ScriptClass> classInstance(Tokens::Iterator & titr,
		const std::shared_ptr<Program> & parent) {
	auto name = titr.get().getValue();
	auto line = titr.get().getLine();
	auto pos = titr.get().getPosition();
	if (titr.accept(Token::Type::IDENT) == false) {
		titr.error("expect: identifier");
	}
	if (titr.accept(Token::Type::BEGIN) == false) {
		titr.error("expect: '{'");
	}
	if (parent->hasMember(name)) {
		titr.error("Class name '" + name + "' shadows existing identifier");
	}

	std::unordered_map<std::string, ScriptObjectPtr> vars;
	std::unordered_map<std::string, std::shared_ptr<Procedure> > instanceFuncs;
	std::vector<ScriptObjectPtr> initElements;

	while (titr.accept(Token::Type::END) == false) {
		if (titr.accept(Token::Type::STATIC_TYPE)) {
			auto s = titr.get();
			if (titr.accept(Token::Type::DEF)) {
				// add function to program variables
				auto name = titr.get().getValue();
				vars[name] = def(titr, parent);
			} else if (titr.accept(Token::Type::IDENT)) {
				// add static variable to program variables
				vars[s.getValue()] = None::none();
				// add code to program initialization
				if (titr.accept(Token::Type::ASSIGN) == false) {
					titr.error("expect: '=='");
				}
				auto expr = expression(titr);
				initElements.insert(initElements.end(), expr.begin(),
						expr.end());
				if (titr.accept(Token::Type::SEMI) == false) {
					titr.error("expect: ';'");
				}
				initElements.emplace_back(
						std::make_shared<String>(s.getValue()));
				initElements.emplace_back(nullptr);
				initElements.emplace_back(
						std::make_shared<Command>("setMember", 3,
								titr.get().getLine(),
								titr.get().getPosition()));
			} else {
				// static initialization block
				auto blk = block(titr);
				initElements.insert(initElements.end(), blk.begin(), blk.end());
			}
		} else {
			if (titr.accept(Token::Type::DEF) == false) {
				titr.error("expect: 'def'");
			}
			// add function to program variables
			auto name = titr.get().getValue();
			instanceFuncs[name] = def(titr, parent);
		}
	}

	auto proc = std::make_shared<Procedure>(parent, line, pos, nullptr,
			initElements);
	auto instance = std::make_shared<ScriptClass>(name, vars, proc,
			instanceFuncs);

	proc->patchInstance(instance);

	return instance;
}

std::vector<ScriptObjectPtr> Parse(const Tokens & tokens) {
	std::vector<ScriptObjectPtr> list;
	try {
		auto titr = tokens.begin();
		while (titr.hasNext()) {
			auto list2 = assignment(titr);
			list.insert(list.end(), list2.begin(), list2.end());
		}
	} catch (std::exception & e) {
		std::cout << e.what() << std::endl;
		list.clear();
	}
	return list;
}
