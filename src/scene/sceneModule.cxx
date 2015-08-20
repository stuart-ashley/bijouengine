#include "sceneModule.h"

#include "labelTask.h"
#include "panelTask.h"
#include "sceneProgramManager.h"
#include "sgAnimator.h"
#include "sgCamera.h"
#include "sgCollision.h"
#include "sgConstraint.h"
#include "sgEdgeIndexArray.h"
#include "sgEndEffector.h"
#include "sgEnvmap.h"
#include "sgIndexedTriangles.h"
#include "sgIrradianceVolume.h"
#include "sgMirror.h"
#include "sgModel.h"
#include "sgNode.h"
#include "sgProjectedKaleidoscope.h"
#include "sgProjectedTexture.h"
#include "sgRigidBody.h"
#include "sgRotate.h"
#include "sgSkinningMatrices.h"
#include "sgShader.h"
#include "sgSunlight.h"
#include "sgTransform.h"
#include "sgTranslate.h"
#include "sgUniform.h"
#include "sgVertexAttribute.h"
#include "sgVolumetricLighting.h"
#include "viewTask.h"

#include "../core/loadManager.h"

#include "../render/font.h"
#include "../render/fontManager.h"

#include "../scripting/bool.h"
#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/path.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <iostream>
#include <stack>

namespace {
	/*
	 *
	 */
	struct Print: public Executable {
		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			std::string str = "";
			for (unsigned i = 0; i < nArgs; ++i) {
				const auto & p = stack.top();
				str += p->toString();
				stack.pop();
			}
			std::cout << str << std::endl;
		}
	};

	/*
	 *
	 */
	struct LoadScript: public Executable {
		std::string currentDir;

		LoadScript(const std::string & currentDir) :
				currentDir(currentDir) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto first = stack.top();
			stack.pop();
			auto cb = stack.top();
			stack.pop();

			std::string canonicalFilename;
			if (typeid(*first) == typeid(String)) {
				auto name = std::static_pointer_cast<String>(first);
				canonicalFilename = LoadManager::getInstance()->getName(
						currentDir, name->getValue());
			} else {
				auto path = std::static_pointer_cast<Path>(first);
				canonicalFilename = path->getCanonicalName();
			}

			SceneProgramManager::getInstance().loadScript(canonicalFilename,
					cb);
		}
	};

	/*
	 *
	 */
	struct Loaded: public Executable {
		std::string currentDir;

		Loaded(const std::string & currentDir) :
				currentDir(currentDir) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto name = getArg<String>("string", stack, 1).getValue();

			auto canonicalFilename = LoadManager::getInstance()->getName(
					currentDir, name);

			if (LoadManager::getInstance()->isLoaded(canonicalFilename)) {
				stack.push(Bool::True());
			} else {
				stack.push(Bool::False());
			}
		}
	};

	/*
	 *
	 */
	struct FontO: public Executable {
		std::string currentDir;

		FontO(const std::string & currentDir) :
				currentDir(currentDir) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto name = getArg<String>("string", stack, 1).getValue();

			stack.push(FontManager::getFont(currentDir, name));
		}
	};

	/*
	 *
	 */
	struct GetCanonicalFilename: public Executable {
		std::string currentDir;

		GetCanonicalFilename(const std::string & currentDir) :
				currentDir(currentDir) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto name = getArg<String>("string", stack, 1).getValue();

			auto canonicalFilename = LoadManager::getInstance()->getName(
					currentDir, name);

			stack.push(std::make_shared<String>(canonicalFilename));
		}
	};
}

/*
 *
 */
SceneModule::SceneModule(const std::string & currentDir) :
		members(std::unordered_map<std::string, ScriptObjectPtr >{
				{ "print", std::make_shared<Print>() },
				{ "loadScript", std::make_shared<LoadScript>(currentDir) },
				{ "loaded", std::make_shared<Loaded>(currentDir) },
				{ "font", std::make_shared<FontO>(currentDir) },
				{ "getCanonicalFilename", std::make_shared<GetCanonicalFilename>(currentDir) },
				{ "LabelTask", LabelTask::getFactory(currentDir) },
				{ "PanelTask",PanelTask::getFactory() },
				{ "ViewTask", ViewTask::getFactory() },
				{ "SgAnimator", SgAnimator::getFactory() },
				{ "SgCamera", SgCamera::getFactory() },
				{ "SgConstraint", SgConstraint::getFactory() },
				{ "SgCollision", SgCollision::getFactory() },
				{ "SgEdgeIndexArray", SgEdgeIndexArray::getFactory(currentDir) },
				{ "SgEndEffector", SgEndEffector::getFactory() },
				{ "SgEnvmap", SgEnvmap::getFactory() },
				{ "SgIndexedTriangles", SgIndexedTriangles::getFactory(currentDir) },
				{ "SgIrradianceVolume", SgIrradianceVolume::getFactory(currentDir) },
				{ "SgMirror", SgMirror::getFactory() },
				{ "SgModel", SgModel::getFactory() },
				{ "SgNode", SgNode::getFactory() },
				{ "SgProjectedKaleidoscope", SgProjectedKaleidoscope::getFactory() },
				{ "SgProjectedTexture", SgProjectedTexture::getFactory() },
				{ "SgRigidBody", SgRigidBody::getFactory() },
				{ "SgRotate", SgRotate::getFactory() },
				{ "SgSkinningMatrices", SgSkinningMatrices::getFactory() },
				{ "SgShader", SgShader::getFactory(currentDir) },
				{ "SgSunlight", SgSunlight::getFactory() },
				{ "SgTransform", SgTransform::getFactory() },
				{ "SgTranslate", SgTranslate::getFactory() },
				{ "SgUniform", SgUniform::getFactory() },
				{ "SgVertexAttribute", SgVertexAttribute::getFactory(currentDir) },
				{ "SgVolumetricLighting", SgVolumetricLighting::getFactory() } }) {
}

/*
 *
 */
SceneModule::~SceneModule() {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr SceneModule::getMember(ScriptExecutionState &,
		const std::string & name) const {
	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return nullptr;
}
