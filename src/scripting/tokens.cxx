#include "tokens.h"

#include "token.h"

#include <cassert>
#include <exception>
#include <iostream>

namespace {

	Token reserved(const std::string & s, size_t line, size_t pos) {
		if (s == "def" || s == "DEF") {
			return Token(Token::Type::DEF, s, line, pos);
		}
		if (s == "if" || s == "IF") {
			return Token(Token::Type::IF, s, line, pos);
		}
		if (s == "else" || s == "ELSE") {
			return Token(Token::Type::ELSE, s, line, pos);
		}
		if (s == "elif" || s == "ELIF") {
			return Token(Token::Type::ELIF, s, line, pos);
		}
		if (s == "for" || s == "FOR") {
			return Token(Token::Type::FOR, s, line, pos);
		}
		if (s == "while" || s == "WHILE") {
			return Token(Token::Type::WHILE, s, line, pos);
		}
		if (s == "return" || s == "RETURN") {
			return Token(Token::Type::RETURN, s, line, pos);
		}
		if (s == "try" || s == "TRY") {
			return Token(Token::Type::TRY, s, line, pos);
		}
		if (s == "catch" || s == "CATCH") {
			return Token(Token::Type::CATCH, s, line, pos);
		}
		if (s == "static" || s == "STATIC") {
			return Token(Token::Type::STATIC_TYPE, s, line, pos);
		}
		if (s == "true" || s == "TRUE") {
			return Token(Token::Type::TRUE, s, line, pos);
		}
		if (s == "false" || s == "FALSE") {
			return Token(Token::Type::FALSE, s, line, pos);
		}
		if (s == "null" || s == "NULL") {
			return Token(Token::Type::NONE, s, line, pos);
		}
		if (s == "class" || s == "CLASS") {
			return Token(Token::Type::CLASS, s, line, pos);
		}
		return Token(Token::Type::IDENT, s, line, pos);
	}

	std::string printChar(char c) {
		if (isprint(c)) {
			return "'" + std::string(1, c) + "'";
		}
		return "'^" + std::string(1, c + '@') + "'";
	}
}

struct Tokens::impl {
	std::string m_str;
	size_t m_length;

	void error(const std::string & msg, size_t line, size_t idx) const {
		std::cerr << "Line " << line << ", " << msg << std::endl;

		size_t lineStart = m_str.rfind('\n', idx > 0 ? idx - 1 : 0);
		if (lineStart == std::string::npos) {
			lineStart = 0;
		} else {
			++lineStart;
		}
		size_t lineEnd = m_str.find('\n', idx);
		if (lineEnd == std::string::npos) {
			lineEnd = m_length;
		}

		std::cerr << m_str.substr(lineStart, lineEnd) << std::endl;

		for (size_t i = 0; i < idx - lineStart; ++i)
			std::cerr << " ";
		std::cerr << "^" << std::endl;

		struct Exception: public std::exception {
			std::string m_msg;

			Exception(const std::string & msg) :
					m_msg(msg) {
			}
			virtual const char * what() const throw () {
				return m_msg.c_str();
			}
		} ex("");
		throw ex;
	}

	Token getString(size_t & line, size_t & idx) const {
		char terminator = m_str[idx];
		++idx;

		std::string s = "";
		for (; idx < m_length; ++idx) {
			char c = m_str[idx];
			if (idx + 1 < m_length && c == '\\') {
				char next = m_str[idx + 1];
				if (next == terminator || next == '\\') {
					s += next;
				} else if (next == 'n') {
					s += '\n';
				} else {
					error("Unrecognized escaped character: " + printChar(next),
							line, idx);
				}
				++idx;
			} else if (c == terminator) {
				++idx;
				return Token(Token::Type::STRING, s, line, idx);
			} else if (isprint(c)) {
				s += c;
			} else {
				error("Unrecognized character: " + printChar(c), line, idx);
			}
		}
		error("Incomplete string: '" + s + "'", line, idx);
		return Token(Token::Type::STRING, s, line, idx);
	}

	Token getNumber(size_t & line, size_t & idx) const {
		std::string s = "";

		char c = m_str[idx];
		if (c == '+' || c == '-') {
			s += c;
			++idx;
		}

		bool dp = false;
		bool exp = false;

		for (; idx < m_length; ++idx) {
			c = m_str[idx];
			if (c >= '0' && c <= '9') {
				s += c;
			} else if (c == '.' && dp == false && exp == false) {
				s += c;
				dp = true;
			} else if (idx + 1 < m_length && (c == 'e' || c == 'E')
					&& exp == false) {
				s += c;
				exp = true;

				char next = m_str[idx + 1];
				if (next == '+' || next == '-') {
					s += next;
					++idx;
				}

				next = m_str[idx + 1];
				if (c < '0' && c > '9') {
					error("Unrecognized character: " + printChar(c), line, idx);
				}
			} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				error("Unrecognized character: " + printChar(c), line, idx);
			} else {
				break;
			}
		}
		return Token(Token::Type::NUMBER, s, line, idx);
	}

	Token getIdentifier(size_t & line, size_t & idx) const {
		std::string s = "";
		for (; idx < m_length; ++idx) {
			char c = m_str[idx];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
					|| (c >= '0' && c <= '9') || c == '_') {
				s += c;
			} else {
				break;
			}
		}
		return reserved(s, line, idx);
	}

	Token getToken(size_t & line, size_t & idx) const {
		assert(idx < m_length);

		char c = m_str[idx];

		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
			return getIdentifier(line, idx);
		} else if (c >= '0' && c <= '9') {
			return getNumber(line, idx);
		} else {
switch (c) {
case '(':
	++idx;
	return Token(Token::Type::LPAREN, "(", line, idx);
case ')':
	++idx;
	return Token(Token::Type::RPAREN, ")", line, idx);
case '{':
	++idx;
	return Token(Token::Type::BEGIN, "{", line, idx);
case '}':
	++idx;
	return Token(Token::Type::END, "}", line, idx);
case '=':
	if (idx + 1 < m_length && m_str.at(idx + 1) == '=') {
		idx += 2;
		return Token(Token::Type::OP, "==", line, idx);
	}
	++idx;
	return Token(Token::Type::ASSIGN, "=", line, idx);
case '.': {
	if (idx + 1 < m_length) {
		char next = m_str.at(idx + 1);
		if (next >= '0' && next <= '9') {
			return getNumber(line, idx);
		}
	}
	++idx;
	return Token(Token::Type::DOT, ".", line, idx);
}
case '+':
case '-': {
	if (idx + 1 < m_length) {
		char next = m_str[idx + 1];
		if (idx + 2 < m_length && next == '.') {
			next = m_str[idx + 2];
		}
		if (next >= '0' && next <= '9') {
			return getNumber(line, idx);
		}
	}
	++idx;
	return Token(Token::Type::OP, std::string(1, c), line, idx);
}
case '/':
case '*':
case '^':
	++idx;
	return Token(Token::Type::OP, std::string(1, c), line, idx);
case ',':
	++idx;
	return Token(Token::Type::COMMA, ",", line, idx);
case ';':
	++idx;
	return Token(Token::Type::SEMI, ";", line, idx);
case ':':
	++idx;
	return Token(Token::Type::COLON, ":", line, idx);
case '!':
case '>':
case '<':
	if (idx + 1 < m_length && m_str[idx + 1] == '=') {
		idx += 2;
		return Token(Token::Type::OP, std::string(1, c) + "=", line,
			idx);
	}
	++idx;
	return Token(Token::Type::OP, std::string(1, c), line, idx);
case '&':
	if (idx + 1 < m_length && m_str[idx + 1] == '&') {
		idx += 2;
		return Token(Token::Type::OP, "&&", line, idx);
	}
	++idx;
	return Token(Token::Type::OP, "&", line, idx);
case '|':
	if (idx + 1 < m_length && m_str[idx + 1] == '|') {
		idx += 2;
		return Token(Token::Type::OP, "||", line, idx);
	}
	++idx;
	return Token(Token::Type::OP, "|", line, idx);
case '\'':
case '"':
	return getString(line, idx);
default:
	error("Tokenization error", line, idx);
	return Token(Token::Type::NONE, "", line, idx);
}
		}
	}

	void skipWhiteSpace(size_t & line, size_t & idx) {
		std::string s = "";
		bool comment = false;
		for (; idx < m_length; ++idx) {
			char c = m_str[idx];
			if (comment) {
				s += c;
				if (c == '\n') {
					++line; // newline
				}
				if (c == '\r') {
					if (idx + 1 < m_length && m_str[idx + 1] == '\n') {
						++idx;
					}
					++line; // newline
				}
				if (c == '*' && m_str[idx + 1] == '/') {
					comment = false;
					s = "";
					++idx;
				}
				continue;
			} else if (c == '\n') {
				++line; // newline
			} else if (c == '\r') {
				if (idx + 1 < m_length && m_str[idx + 1] == '\n') {
					++idx;
				}
				++line; // newline
			} else if (c == ' ' || c == '\t') {
				// whitespace
			} else if (c == '/' && m_str[idx + 1] == '*') {
				comment = true;
				idx += 2;
			} else {
				return;
			}
		}
		if (s.length() > 0) {
			error("Remaining: '" + s + "'", line, m_length);
		}
	}

	impl(const std::string & str) :
			m_str(str), m_length(str.length()) {
	}

	void dump() {
		size_t line = 1;
		size_t idx = 0;
		skipWhiteSpace(line, idx);
		while (idx < m_length) {
			std::cout << getToken(line, idx).getValue() << ",";
			skipWhiteSpace(line, idx);
		}
		std::cout << std::endl;
	}
}
;

/*
 */
Tokens::Tokens(const std::string & str) :
		pimpl(new impl(str)) {
}

/*
 */
Tokens::~Tokens() {
}

/*
 */
Tokens::Iterator Tokens::begin() const {
	return Tokens::Iterator(*this);
}

/*
 */
Tokens::Iterator::Iterator(const Tokens & tokens) :
		m_parent(tokens), m_line(1), m_idx(0) {
	m_parent.pimpl->skipWhiteSpace(m_line, m_idx);
}

/*
 */
Token Tokens::Iterator::get() const {
	size_t line = m_line;
	size_t idx = m_idx;
	return m_parent.pimpl->getToken(line, idx);
}

/*
 */
bool Tokens::Iterator::hasNext() const {
	return m_idx < m_parent.pimpl->m_length;
}

/*
 */
bool Tokens::Iterator::accept(Token::Type type) {
	if (m_idx < m_parent.pimpl->m_length) {
		size_t line = m_line;
		size_t idx = m_idx;
		if (m_parent.pimpl->getToken(line, idx).isType(type)) {
			m_parent.pimpl->skipWhiteSpace(line, idx);
			m_line = line;
			m_idx = idx;
			return true;
		}
	}
	return false;
}

/*
 */
bool Tokens::Iterator::accept(Token::Type type, Token::Type type2) {
	if (m_idx < m_parent.pimpl->m_length) {
		size_t line = m_line;
		size_t idx = m_idx;
		if (m_parent.pimpl->getToken(line, idx).isType(type)) {
			m_parent.pimpl->skipWhiteSpace(line, idx);
			if (idx < m_parent.pimpl->m_length) {
				if (m_parent.pimpl->getToken(line, idx).isType(type2)) {
					m_parent.pimpl->skipWhiteSpace(line, idx);
					m_line = line;
					m_idx = idx;
					return true;
				}
			}
		}
	}
	return false;
}

/*
 */
void Tokens::Iterator::error(const std::string & msg) const {
	m_parent.pimpl->error(msg, m_line, m_idx);
}
