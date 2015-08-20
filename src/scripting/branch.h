#pragma once

#include "scriptObject.h"

class Branch: public ScriptObject {
public:
	enum Type {
		B, BIF, BNIF
	};

	Branch(Type type, int offset);
	~Branch();

	Type getType() const;
	bool isType(Branch::Type type) const;
	int getOffset() const;

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
