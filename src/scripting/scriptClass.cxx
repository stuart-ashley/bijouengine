#include "scriptClass.h"

#include "breakpointMarker.h"
#include "classInstance.h"
#include "procedure.h"

struct ScriptClass::impl {
	std::string m_className;
	/** initialization code */
	std::shared_ptr<Procedure> m_initProc;
	/** instance functions */
	std::unordered_map<std::string, std::shared_ptr<Procedure> > m_instanceFunctions;
	std::unordered_map<std::string, ScriptObjectPtr> m_members;

	impl(const std::string & className,
			const std::unordered_map<std::string, ScriptObjectPtr> & members,
			const std::shared_ptr<Procedure> & initProc,
			const std::unordered_map<std::string, std::shared_ptr<Procedure> > & instanceFunctions) :
					m_className(className),
					m_initProc(initProc),
					m_instanceFunctions(instanceFunctions),
					m_members(members) {
	}
};

/**
 *
 * @param className
 * @param members
 * @param initProc
 * @param instanceFunctions
 */
ScriptClass::ScriptClass(const std::string & className,
		const std::unordered_map<std::string, ScriptObjectPtr> & members,
		const std::shared_ptr<Procedure> & initProc,
		const std::unordered_map<std::string, std::shared_ptr<Procedure> > & instanceFunctions) :
		pimpl(new impl(className, members, initProc, instanceFunctions)) {
}

/**
 * destructor
 */
ScriptClass::~ScriptClass() {
}

/**
 *
 * @return
 */
std::vector<std::shared_ptr<BreakpointMarker> > ScriptClass::getBreakpoints() const {
	auto breakpoints = pimpl->m_initProc->getBreakpoints();
	for (const auto & e : pimpl->m_instanceFunctions) {
		auto bps = e.second->getBreakpoints();
		breakpoints.insert(breakpoints.begin(), bps.begin(), bps.end());
	}
	for (const auto & e : pimpl->m_members) {
		if (typeid(*e.second) == typeid(Procedure)) {
			auto proc = std::static_pointer_cast<Procedure>(e.second);
			auto bps = proc->getBreakpoints();
			breakpoints.insert(breakpoints.begin(), bps.begin(), bps.end());
		}
	}
	return breakpoints;
}

/**
 *
 * @return
 */
const std::string & ScriptClass::getClassName() const {
	return pimpl->m_className;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr ScriptClass::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	const auto & e = pimpl->m_members[name];
	if (e != nullptr) {
		return e;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @param execState
 */
void ScriptClass::init(ScriptExecutionState & execState) {
	std::stack<ScriptObjectPtr> stack;
	pimpl->m_initProc->execProc(execState, nullptr, 0, stack);
}

/**
 *
 * @return
 */
ScriptObjectPtr ScriptClass::newInstance() const {
	auto instance = std::make_shared<ClassInstance>(pimpl->m_className);
	for (const auto & e : pimpl->m_instanceFunctions) {
		instance->setMember(e.first, e.second);
	}
	return instance;
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
OVERRIDE void ScriptClass::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	auto result = pimpl->m_members.emplace(name, value);
	if (result.second == false) {
		result.first->second = value;
	}
}

