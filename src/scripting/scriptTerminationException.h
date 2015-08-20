#pragma once

#include <exception>

class ScriptTerminationException: std::exception {
public:
	virtual const char * what() const throw () {
		return "Script termination";
	}
};
