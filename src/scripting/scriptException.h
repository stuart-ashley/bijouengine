#pragma once

#include <exception>
#include <memory>
#include <string>

class ScriptException: public std::exception {
public:
	ScriptException(const std::string & message, const std::string & filename,
			int line, int pos);

	ScriptException(const ScriptException & other);

	~ScriptException() throw ();

	const char * what() const throw () override;

	const std::string & getMessage() const;
	const std::string & getFilename() const;
	int getLine() const;
	int getPosition() const;

	std::string toString() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
