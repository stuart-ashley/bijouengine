#include "sgTranslate.h"

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

			auto translation = getArg<Vec3>("vec3", stack, 1);

			stack.push(std::make_shared<SgTranslate>(translation));
		}
	};

	struct GetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgTranslate = std::static_pointer_cast<SgTranslate>(self);

			stack.push(std::make_shared<Vec3>(sgTranslate->getTranslation()));
		}
	};

	struct SetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgTranslate = std::static_pointer_cast<SgTranslate>(self);

			auto translation = getArg<Vec3>("vec3", stack, 1);

			sgTranslate->setTranslation(translation);
		}
	};

	struct Translate: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgTranslate = std::static_pointer_cast<SgTranslate>(self);

			auto translation = getArg<Vec3>("vec3", stack, 1);

			sgTranslate->setTranslation(
					sgTranslate->getTranslation() + translation);
		}
	};
}

SgTranslate::SgTranslate(const Vec3 & translation) :
		m_translation(translation) {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgTranslate::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = { {
			"getTranslation", std::make_shared<GetTranslation>() }, {
			"setTranslation", std::make_shared<SetTranslation>() }, {
			"translate", std::make_shared<Translate>() } };

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
Vec3 SgTranslate::getTranslation() const {
	return m_translation;
}

/**
 *
 * @param translation
 */
void SgTranslate::setTranslation(const Vec3 & translation) {
	m_translation = translation;
}

/**
 *
 * @param state
 */
OVERRIDE void SgTranslate::update(UpdateState & state) {
	state.translate(m_translation);
}

/**
 *
 * @param builder
 */
OVERRIDE void SgTranslate::taskInit(Builder & builder) {
	builder.translate(m_translation);
}

/**
 *
 * @param vb
 */
OVERRIDE void SgTranslate::visualize(render::ViewBuilder & vb) {
	vb.getState().translate(m_translation);
}

/**
 * get script object factory for SgTranslate
 *
 * @return  SgTranslate factory
 */
STATIC const ScriptObjectPtr & SgTranslate::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
