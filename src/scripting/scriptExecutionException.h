#pragma once

#include <exception>
#include <string>

class ScriptExecutionException: public std::exception {
public:
	std::string m_msg;

	ScriptExecutionException(const std::string & msg) :
			m_msg(msg) {
	}

	virtual const char * what() const throw () {
		return m_msg.c_str();
	}
};

inline void scriptExecutionAssert(bool assertion, const std::string & msg) {
	if (assertion == false) {
		throw ScriptExecutionException(msg);
	}
}

template<typename TYPE>
void scriptExecutionAssertType(const ScriptObjectPtr & e, const std::string & msg) {
	scriptExecutionAssert(typeid( *e ) == typeid(TYPE), msg);
}
