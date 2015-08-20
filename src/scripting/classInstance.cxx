#include "classInstance.h"

#include "executable.h"
#include "map.h"
#include "parameters.h"
#include "scriptExecutionException.h"
#include "string.h"

struct ClassInstance::impl {
	struct GetMembers: public Executable {

		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto instance = std::static_pointer_cast<ClassInstance>(self);

			const auto & members = instance->getMembers();

			auto map = std::make_shared<Map>();

			for (const auto & entry : members) {
				auto key = std::make_shared<String>(entry.first);
				map->put(key, entry.second);
			}

			stack.emplace(map);
		}
	};

	std::string className;
	std::unordered_map<std::string, ScriptObjectPtr> members;

	impl(const std::string & className) :
			className(className) {
		members["__type__"] = std::make_shared<String>(className);

		members["getMembers"] = std::make_shared<GetMembers>();
	}
};

/*
 *
 */
ClassInstance::ClassInstance(const std::string & className) :
		pimpl(new impl(className)) {
}

/*
 *
 */
ClassInstance::~ClassInstance() {
}

/*
 *
 */
const std::unordered_map<std::string, ScriptObjectPtr> & ClassInstance::getMembers() const {
	return pimpl->members;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr ClassInstance::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	auto entry = pimpl->members.find(name);
	if (entry != pimpl->members.end()) {
		return entry->second;
	}
	try {
		return ScriptObject::getMember(execState, name);
	} catch (ScriptExecutionException & e) {
		throw ScriptExecutionException(
				"Can't get member '" + name + "' of class '" + pimpl->className
						+ "'");
	}
	return nullptr;
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
OVERRIDE void ClassInstance::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	auto result = pimpl->members.emplace(name, value);
	if (result.second == false) {
		result.first->second = value;
	}
}
