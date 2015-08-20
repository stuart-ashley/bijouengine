#pragma once

#include "bool.h"
#include "parameter.h"
#include "real.h"
#include "scriptExecutionException.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <deque>
#include <string>
#include <stack>

class Parameters {
public:
	Parameters(const std::vector<BaseParameter> & parameters);
	~Parameters();

	std::unordered_map<std::string, ScriptObjectPtr> getArgs(unsigned int nArgs,
			std::stack<ScriptObjectPtr> & stack) const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

inline void checkNumArgs(unsigned nArgs, unsigned required) {
	if (nArgs != required) {
		throw ScriptExecutionException(
				"Require " + std::to_string(required)
						+ (required == 1 ? " argument" : " arguments") + " got "
						+ std::to_string(nArgs));
	}
}

template<typename TYPE>
inline TYPE getArg(const std::string & type,
		std::stack<ScriptObjectPtr> & stack, int argNum) {
	auto arg = stack.top();
	stack.pop();

	if (typeid(*arg) != typeid(TYPE)) {
		throw ScriptExecutionException(
				"Require " + type + " for argument " + std::to_string(argNum));
	}

	return *std::static_pointer_cast<TYPE>(arg);
}

inline bool getBoolArg(std::stack<ScriptObjectPtr> & stack, int argNum) {
	auto arg = stack.top();
	stack.pop();

	if (typeid(*arg) != typeid(Bool)) {
		throw ScriptExecutionException(
				"Require boolean for argument " + argNum);
	}

	return arg == Bool::True();
}

inline int16_t getInt16Arg(std::stack<ScriptObjectPtr> & stack, int argNum) {
	auto arg = stack.top();
	stack.pop();

	if (typeid(*arg) != typeid(Real)) {
		throw ScriptExecutionException(
				"Require 16 bit integer for argument " + argNum);
	}

	auto num = std::static_pointer_cast<Real>(arg);
	if (num->isInt16() == false) {
		throw ScriptExecutionException(
				"Require 16 bit integer for argument " + argNum);
	}

	return num->getInt16();
}

inline int32_t getInt32Arg(std::stack<ScriptObjectPtr> & stack, int argNum) {
	auto arg = stack.top();
	stack.pop();

	if (typeid(*arg) != typeid(Real)) {
		throw ScriptExecutionException(
				"Require 32 bit integer for argument " + argNum);
	}

	auto num = std::static_pointer_cast<Real>(arg);
	if (num->isInt32() == false) {
		throw ScriptExecutionException(
				"Require 32 bit integer for argument " + argNum);
	}

	return num->getInt32();
}

#define getNumericArg(x,y) __getNumericArg(x,y,__FILE__,__LINE__)

inline double __getNumericArg(std::stack<ScriptObjectPtr> & stack, int argNum,const char * file, int line) {
	auto arg = stack.top();
	stack.pop();

	if (typeid(*arg) != typeid(Real)) {
		throw ScriptExecutionException(std::string(file)+":"+std::to_string(line)+": "
				"Require numeric argument " + std::to_string(argNum) + " got "
						+ std::string(typeid(*arg).name()));
	}

	return std::static_pointer_cast<Real>(arg)->getValue();
}

inline float getFloatArg(std::stack<ScriptObjectPtr> & stack, int argNum){
	auto arg = stack.top();
	stack.pop();

	if (typeid(*arg) != typeid(Real)) {
		throw ScriptExecutionException(
			"Require 16 bit integer for argument " + argNum);
	}
	return std::static_pointer_cast<Real>(arg)->getFloat();
}
