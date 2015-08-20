#pragma once

#include <memory>
#include "scriptObject.h"

class None: public ScriptObject {
public:
	~None();

	static std::shared_ptr<None> none();
private:
	None();
};

