#include "placeholder.h"

struct Placeholder::impl {
	std::string m_name;
	int m_line;
	int m_position;

	impl(const std::string & name, int line, int position) :
			m_name(name), m_line(line), m_position(position) {
	}
};

/*
 *
 */
Placeholder::Placeholder(const std::string & name, int line, int position) :
		pimpl(new impl(name, line, position)) {
}

/*
 *
 */
Placeholder::~Placeholder() {
}

/*
 *
 */
const std::string & Placeholder::getName() const {
	return pimpl->m_name;
}

/*
 *
 */
int Placeholder::getLine() const {
	return pimpl->m_line;
}

/*
 *
 */
int Placeholder::getPosition() const {
	return pimpl->m_position;
}
