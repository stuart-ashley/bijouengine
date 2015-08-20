#pragma once

#include "token.h"

#include <memory>
#include <string>

class Tokens {
public:
	class Iterator {
	private:
		const Tokens & m_parent;
		size_t m_line;
		size_t m_idx;

	public:
		Iterator(const Tokens & tokens);

		Token get() const;

		bool hasNext() const;

		bool accept(Token::Type type);
		bool accept(Token::Type type, Token::Type type2);

		void error(const std::string & msg) const;
	};

	Tokens(const std::string & str);
	~Tokens();

	Iterator begin() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
