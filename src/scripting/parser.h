#pragma once

#include "scriptObject.h"
#include "tokens.h"

#include <deque>
#include <memory>
#include <vector>

class ScriptClass;
class Program;
class Procedure;

ScriptObjectPtr constTerm(Tokens::Iterator & titr);

std::deque<ScriptObjectPtr> expression(Tokens::Iterator & titr);

std::deque<ScriptObjectPtr> statement(Tokens::Iterator & titr);

std::shared_ptr<Procedure> def(Tokens::Iterator & titr,
		const std::shared_ptr<Program> & parent);

std::shared_ptr<ScriptClass> classInstance(Tokens::Iterator & titr,
		const std::shared_ptr<Program> & parent);

std::vector<ScriptObjectPtr> Parse(const Tokens & tokens);
