#include "parameters.h"

#include "kwarg.h"
#include "parameter.h"
#include "scriptExecutionException.h"

#include <algorithm>

struct Parameters::impl {
	std::vector<BaseParameter> m_parameters;
	std::vector<std::string> m_parameterNames;
	std::unordered_map<std::string, ScriptObjectPtr> m_defaultParameters;
	int m_defaultMask;

	impl(const std::vector<BaseParameter> & parameters) :
			m_parameters(parameters), m_defaultMask(0) {
		for (size_t i = 0, n = parameters.size(); i < n; ++i) {
			auto & parameter = parameters.at(i);
			auto & name = parameter.getName();
			auto value = parameter.getValue();
			m_parameterNames.emplace_back(name);
			m_defaultParameters[name] = value;
			if (value != nullptr) {
				m_defaultMask |= 1 << i;
			}
		}
	}
};

/*
 *
 */
Parameters::Parameters(const std::vector<BaseParameter> & parameters) :
		pimpl(new impl(parameters)) {
}

/*
 *
 */
Parameters::~Parameters() {
}

/*
 *
 */
std::unordered_map<std::string, ScriptObjectPtr> Parameters::getArgs(
		unsigned nArgs, std::stack<ScriptObjectPtr> & stack) const {
	bool keywords = false;
	scriptExecutionAssert(nArgs <= pimpl->m_parameters.size(),
			"Require " + std::to_string(pimpl->m_parameters.size())
					+ " arguments got " + std::to_string(nArgs));

	// use default parameters as args
	auto args = pimpl->m_defaultParameters;
	int mask = pimpl->m_defaultMask;
	// args
	for (unsigned i = 0; i < nArgs; ++i) {
		// arg
		auto arg = stack.top();
		stack.pop();

		if (typeid(*arg) == typeid(Kwarg)) {
			keywords = true;

			// keyword name
			const auto & name =
					(std::static_pointer_cast<Kwarg>(arg))->getKey();

			// find keyword in parameters
			const auto begin = pimpl->m_parameterNames.cbegin();
			const auto end = pimpl->m_parameterNames.cend();
			const auto it = std::find(begin, end, name);
			scriptExecutionAssert(it != end,
					"Unknown parameter '" + name + "'");

			// value
			args[name] = stack.top();
			stack.pop();

			// update mask
			mask |= 1 << std::distance(begin, it);
		} else if (keywords == false) {
			// value
			args[pimpl->m_parameterNames.at(i)] = arg;

			// update mask
			mask |= 1 << i;
		} else {
			scriptExecutionAssert(false, "Non-keyword arg after keyword arg");
		}
	}
	// check
	if (mask != (1 << nArgs) - 1) {
		for (auto entry : args) {
			scriptExecutionAssert(entry.second != nullptr,
					"Missing argument for parameter '" + entry.first + "'");
		}
	}
	return args;
}

