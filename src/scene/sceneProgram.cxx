#include "sceneProgram.h"

#include "sceneModule.h"
#include "system.h"
#include "updateState.h"

#include "../core/coreModule.h"
#include "../core/loadingCallback.h"
#include "../core/loadManager.h"
#include "../core/loadManagerUtils.h"

#include "../render/renderModule.h"

#include "../scripting/functor.h"
#include "../scripting/procedure.h"
#include "../scripting/program.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/scriptExecutionState.h"

#include <cassert>
#include <iostream>
#include <stack>

using namespace render;

namespace {
	struct LoadScript: public LoadingCallback {
		std::shared_ptr<Program> & parent;
		std::shared_ptr<Program> prog;

		LoadScript(std::shared_ptr<Program> & parent,
				const std::string & canonicalFilename) :
						LoadingCallback("", canonicalFilename),
						parent(parent),
						prog(nullptr) {
			LoadManager::getInstance()->load(*this);
		}

		void load(std::istream & stream) override {
			static auto systemInstance = std::make_shared<System>();

			const auto & filename = getCanonicalFilename();
			prog = Program::create(filename, stream);

			auto currentDir = LoadManagerUtils::getDirectory(filename);

			prog->use(std::make_shared<CoreModule>(currentDir));
			prog->use(std::make_shared<RenderModule>(currentDir));
			prog->use(std::make_shared<SceneModule>(currentDir));

			prog->setMember("System", systemInstance);
			parent = prog;
		}
	};

	std::string emptyContent = "";
}

/*
 *
 */
SceneProgram::SceneProgram(const std::string & canonicalFilename) {
	LoadScript loadScript(program, canonicalFilename);
}

/*
 *
 */
void SceneProgram::addLoadedCallback(const ScriptObjectPtr & callback) {
	callbacks.emplace_back(callback);
}

/*
 *
 */
void SceneProgram::execute(const std::shared_ptr<UpdateState> & state) const {
	if (program != nullptr) {
		ScriptExecutionState execState;
		ScriptObjectPtr e;
		try {
			e = getMember(execState, "update");
		} catch (ScriptExecutionException & e1) {
			std::cerr << "Can't execute update" << std::endl;
			assert(false);
		}
		state->pushState();
		std::stack<ScriptObjectPtr> stack;
		stack.push(state);
		std::static_pointer_cast<Procedure>(e)->execProc(execState, nullptr, 1,
				stack);
		state->popState();
	}
}

/*
 *
 */
void SceneProgram::executeLoadedCallbacks() {
	if (program == nullptr) {
		return;
	}

	for (const auto & callback : callbacks) {
		ScriptExecutionState execState;
		std::stack<ScriptObjectPtr> stack;
		stack.push(std::make_shared<SceneProgram>(*this));
		if (typeid(*callback) == typeid(Functor)) {
			std::static_pointer_cast<Functor>(callback)->exec(execState, 1,
					stack);
		} else {
			std::static_pointer_cast<Procedure>(callback)->execProc(execState,
					nullptr, 1, stack);
		}
	}
	callbacks.clear();
}

/*
 *
 */
std::shared_ptr<BreakpointMarker> SceneProgram::getBreakpoint(int line) const {
	if (program == nullptr) {
		return nullptr;
	}
	return program->getBreakpoint(line);
}

/*
 *
 */
std::vector<std::shared_ptr<BreakpointMarker>> SceneProgram::getBreakpoints() const {
	static std::vector<std::shared_ptr<BreakpointMarker>> empty;
	if (program == nullptr) {
		return empty;
	}
	return program->getBreakpoints();

}

/*
 *
 */
const std::string & SceneProgram::getContent() const {
	if (program != nullptr) {
		return program->getContent();
	}
	return emptyContent;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr SceneProgram::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	if (program != nullptr) {
		program->init(execState);
		return program->getMember(execState, name);
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * set named script object member
 *
 * @param name   name of member
 * @param value  desired value
 */
OVERRIDE void SceneProgram::setMember(const std::string & name,
		const ScriptObjectPtr & value) {
	program->setMember(name, value);
}

/*
 *
 */
bool SceneProgram::valid() const {
	return program != nullptr;
}
