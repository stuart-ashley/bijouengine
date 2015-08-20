#include "pair.h"

#include "executable.h"
#include "parameters.h"
#include "scriptExecutionException.h"

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto key = stack.top();
			stack.pop();

			auto value = stack.top();
			stack.pop();

			stack.emplace(std::make_shared<Pair>(key, value));
		}
	};
}

/**
 * get script object factory for Pair
 *
 * @return  Pair factory
 */
const ScriptObjectPtr & Pair::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
