#pragma once

#include <memory>
#include <string>

class Token {
public:
	enum class Type {
		STATIC_TYPE, CLASS, DEF, IF, ELSE, ELIF, FOR, WHILE, RETURN, TRY, CATCH,
		IDENT, STRING, NUMBER, LPAREN, RPAREN, BEGIN, END, ASSIGN, DOT, COMMA,
		SEMI, COLON, OP, TRUE, FALSE, NONE
	};

	Token(Type type, const std::string & value, size_t line, size_t position);
	Token(const Token & other);
	~Token();

	Type getType() const;
	bool isType(Token::Type type) const;
	std::string getValue() const;
	size_t getLine() const;
	size_t getPosition() const;

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
