#include "command.h"

struct Command::impl {
	std::string m_name;
	int m_nParams;
	int m_line;
	int m_position;

	/*
	 * constructor
	 */
	impl(const std::string & name, int nParams, int line, int position) :
			m_name(name), m_nParams(nParams), m_line(line), m_position(position) {
	}
};

/*
 */
Command::Command(const std::string & name, int nParams, int line, int position) :
		pimpl(new impl(name, nParams, line, position)) {
}

/*
 */
Command::~Command() {
}

/*
 */
const std::string & Command::getName() const {
	return pimpl->m_name;
}

/*
 */
int Command::getNumParameters() const {
	return pimpl->m_nParams;
}

/*
 *
 */
int Command::getLine() const {
	return pimpl->m_line;
}

/*
 *
 */
int Command::getPosition() const {
	return pimpl->m_position;
}
