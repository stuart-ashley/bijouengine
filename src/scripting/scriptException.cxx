#include "scriptException.h"

struct ScriptException::impl {
	std::string m_message;
	std::string m_filename;
	int m_line;
	int m_position;

	impl(const std::string & message, const std::string & filename, int line,
			int position) :
					m_message(message),
					m_filename(filename),
					m_line(line),
					m_position(position) {
	}

	impl(const impl & other) :
					m_message(other.m_message),
					m_filename(other.m_filename),
					m_line(other.m_line),
					m_position(other.m_position) {
	}
};

/*
 *
 */
ScriptException::ScriptException(const std::string & message,
		const std::string & filename, int line, int position) :
		pimpl(new impl(message, filename, line, position)) {
}

/*
 *
 */
ScriptException::ScriptException(const ScriptException & other) :
		pimpl(new impl(*other.pimpl)) {
}

/*
 *
 */
ScriptException::~ScriptException() throw () {
}

/*
 *
 */
const char * ScriptException::what() const throw () {
	return pimpl->m_message.c_str();
}

/*
 *
 */
const std::string & ScriptException::getMessage() const {
	return pimpl->m_message;
}

/*
 *
 */
const std::string & ScriptException::getFilename() const {
	return pimpl->m_filename;
}

/*
 *
 */
int ScriptException::getLine() const {
	return pimpl->m_line;
}

/*
 *
 */
int ScriptException::getPosition() const {
	return pimpl->m_position;
}

/*
 *
 */
std::string ScriptException::toString() const {
	return pimpl->m_filename + ":" + std::to_string(pimpl->m_line) + ":"
			+ std::to_string(pimpl->m_position) + ":" + pimpl->m_message;
}
