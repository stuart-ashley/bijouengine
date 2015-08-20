#include "none.h"

None::None() {
}

None::~None() {
}

std::shared_ptr<None> None::none() {
	static std::shared_ptr<None> NONE(new None());
	return NONE;
}
