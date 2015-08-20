#include "token.h"

#include <string>

struct Token::impl {
	Type m_type;
	std::string m_value;
	size_t m_line;
	size_t m_position;

	/*
	 */
	impl(Type type, const std::string & value, size_t line, size_t position) :
			m_type(type), m_value(value), m_line(line), m_position(position) {
	}

	/*
	 */
	impl(const impl & other) :
					m_type(other.m_type),
					m_value(other.m_value),
					m_line(other.m_line),
					m_position(other.m_position) {
	}

};

/*
 */
Token::Token(Type type, const std::string & value, size_t line, size_t position) :
		pimpl(new impl(type, value, line, position)) {
}

/*
 */
Token::Token(const Token & other) :
		pimpl(new impl(*other.pimpl)) {
}

/*
 */
Token::~Token() {
}

/*
 */
Token::Type Token::getType() const {
	return pimpl->m_type;
}

/*
 */
bool Token::isType(Token::Type type) const {
	return pimpl->m_type == type;
}

/*
 */
std::string Token::getValue() const {
	return pimpl->m_value;
}

/*
 */
size_t Token::getLine() const {
	return pimpl->m_line;
}

/*
 */
size_t Token::getPosition() const {
	return pimpl->m_position;
}
