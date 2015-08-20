#include "function.h"

struct Function::impl {
	std::string m_name;
	int m_nParams;
	int m_line;
	int m_position;

	/*
	 */
	impl(const std::string & name, int nParams, int line, int position) :
			m_name(name), m_nParams(nParams), m_line(line), m_position(position) {
	}
};

/*
 */
Function::Function(const std::string & name, int nParams, int line,
		int position) :
		pimpl(new impl(name, nParams, line, position)) {
}

/*
 */
Function::~Function() {
}

/*
 */
const std::string & Function::getName() const {
	return pimpl->m_name;
}

/*
 */
int Function::getNumParameters() const {
	return pimpl->m_nParams;
}

/*
 *
 */
int Function::getLine() const {
	return pimpl->m_line;
}

/*
 *
 */
int Function::getPosition() const {
	return pimpl->m_position;
}
