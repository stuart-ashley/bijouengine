#include "sgRotate.h"

#include "builder.h"
#include "updateState.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

namespace {
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto rotation = getArg<Quat>("quat", stack, 1);

			stack.push(std::make_shared<SgRotate>(rotation));
		}
	};

	struct GetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgRotate = std::static_pointer_cast<SgRotate>(self);

			stack.push(std::make_shared<Quat>(sgRotate->getRotation()));
		}
	};

	struct SetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgRotate = std::static_pointer_cast<SgRotate>(self);

			auto rotation = getArg<Quat>("quat", stack, 1);

			sgRotate->setRotation(rotation);
		}
	};
}

/**
 *
 * @param rotation
 */
SgRotate::SgRotate(const Quat & rotation) :
		m_rotation(rotation) {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgRotate::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = { {
			"getRotation", std::make_shared<GetRotation>() }, { "setRotation",
			std::make_shared<SetRotation>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @return
 */
Quat SgRotate::getRotation() const {
	return m_rotation;
}

/**
 *
 * @param rotation
 */
void SgRotate::setRotation(const Quat & rotation) {
	m_rotation = rotation;
}

/**
 *
 * @param builder
 */
OVERRIDE void SgRotate::taskInit(Builder & builder) {
	builder.rotate(m_rotation);
}

/**
 *
 * @param state
 */
OVERRIDE void SgRotate::update(UpdateState & state) {
	state.rotate(m_rotation);
}

/**
 *
 * @param vb
 */
OVERRIDE void SgRotate::visualize(render::ViewBuilder & vb) {
	vb.getState().rotate(m_rotation);
}

/**
 * get script object factory for SgRotate
 *
 * @return  SgRotate factory
 */
STATIC const ScriptObjectPtr & SgRotate::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
