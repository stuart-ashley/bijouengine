#include "sgCamera.h"

#include "builder.h"

#include "../core/aspect.h"
#include "../core/color.h"
#include "../core/config.h"
#include "../core/debugGeometry.h"
#include "../core/perspectiveCamera.h"
#include "../core/quat.h"
#include "../core/vec3.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <vector>

using namespace render;

namespace {
	std::vector<BaseParameter> params = { Parameter<String>("name", nullptr),
			Parameter<Real>("near", std::make_shared<Real>(.1)),
			Parameter<Real>("far", std::make_shared<Real>(1000)),
			Parameter<Real>("fovInDegrees", std::make_shared<Real>(60)),
			Parameter<Real>("aspectRatio", std::make_shared<Real>(1)) };
	/*
	 *
	 */
	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			const auto & name =
					std::static_pointer_cast<String>(args["name"])->getValue();

			auto near =
					std::static_pointer_cast<Real>(args["near"])->getFloat();

			float far = std::static_pointer_cast<Real>(args["far"])->getFloat();

			float fovInDegrees = std::static_pointer_cast<Real>(
					args["fovInDegrees"])->getFloat();

			float aspectRatio = std::static_pointer_cast<Real>(
					args["aspectRatio"])->getFloat();

			stack.push(
					std::make_shared<SgCamera>(
							std::make_shared<PerspectiveCamera>(name,
									fovInDegrees, aspectRatio, near, far)));
		}
	};

	struct GetAspectRatio: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto aspectRatio =
					std::static_pointer_cast<SgCamera>(self)->getCamera()->getAspectRatio();

			stack.push(std::make_shared<Real>(aspectRatio));
		}
	};

	struct GetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto cam = std::static_pointer_cast<SgCamera>(self);

			stack.push(
					std::make_shared<Quat>(
							cam->getAspect()->getRotTrans().getRotation()));
		}
	};

	struct GetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto cam = std::static_pointer_cast<SgCamera>(self);

			stack.push(
					std::make_shared<Vec3>(
							cam->getAspect()->getRotTrans().getTranslation()));
		}
	};

	struct SetAspectRatio: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgCamera = std::static_pointer_cast<SgCamera>(self);
			auto camera = sgCamera->getCamera();

			float aspectRatio = static_cast<float>(getNumericArg(stack, 1));

			sgCamera->setCamera(
					std::make_shared<PerspectiveCamera>(camera->getName(),
							camera->getFovyInDegrees(), aspectRatio,
							camera->getNear(), camera->getFar()));
		}
	};

	struct Valid: public Executable {

		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto cam = std::static_pointer_cast<SgCamera>(self);

			if (cam->getAspect() != nullptr) {
				stack.push(Bool::True());
			} else {
				stack.push(Bool::False());
			}
		}
	};
}

/**
 *
 * @param cam
 */
SgCamera::SgCamera(const std::shared_ptr<PerspectiveCamera> & camera) :
		m_camera(camera) {
}

/**
 *
 * @return
 */
const std::shared_ptr<Aspect> & SgCamera::getAspect() const {
	return m_aspect;
}

/**
 *
 * @return
 */
const std::shared_ptr<PerspectiveCamera> & SgCamera::getCamera() const {
	return m_camera;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgCamera::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = { {
			"getAspectRatio", std::make_shared<GetAspectRatio>() }, {
			"getRotation", std::make_shared<GetRotation>() }, {
			"getTranslation", std::make_shared<GetTranslation>() }, {
			"setAspectRatio", std::make_shared<SetAspectRatio>() }, { "valid",
			std::make_shared<Valid>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @param camera
 */
void SgCamera::setCamera(const std::shared_ptr<PerspectiveCamera> & camera) {
	m_camera = camera;
}

/**
 *
 * @param builder
 */
OVERRIDE void SgCamera::taskInit(Builder & builder) {
	m_aspect = std::make_shared<Aspect>(m_camera, builder.getTransform());
}

/**
 *
 * @param vb
 */
OVERRIDE void SgCamera::visualize(render::ViewBuilder & vb) {
	if (Config::getInstance().getBoolean("debugCamera")) {
		/* debug bounds */
		std::vector<DebugGeometry> debug;
		debug.emplace_back(vb.getState().getTransform(), Color::red(),
				m_aspect->getConvexHull().getTriangleList());
		vb.addDebugGeometry(debug);
	}
}

/**
 * get script object factory for SgCamera
 *
 * @return  SgCamera factory
 */
STATIC const ScriptObjectPtr & SgCamera::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
