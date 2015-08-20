#define _USE_MATH_DEFINES
#include "sgTransform.h"

#include "builder.h"
#include "updateState.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace {
	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<Real>("pitch", nullptr),
			Parameter<Real>("yaw", nullptr),
			Parameter<Vec3>("translation", nullptr) };

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

			float pitch =
					std::static_pointer_cast<Real>(args["pitch"])->getFloat();
			float yaw = std::static_pointer_cast<Real>(args["yaw"])->getFloat();
			auto translation = std::static_pointer_cast<Vec3>(
					args["translation"]);

			stack.push(std::make_shared<SgTransform>(pitch, yaw, *translation));
		}
	};
	/*
	 *
	 */
	struct GetPitch: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			stack.emplace(std::make_shared<Real>(sgTransform->getPitch()));
		}
	};

	/*
	 *
	 */
	struct GetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			stack.emplace(std::make_shared<Quat>(sgTransform->getRotation()));
		}
	};

	/*
	 *
	 */
	struct GetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			stack.emplace(
					std::make_shared<Vec3>(sgTransform->getTranslation()));
		}
	};

	/*
	 *
	 */
	struct GetYaw: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			stack.emplace(std::make_shared<Real>(sgTransform->getYaw()));
		}
	};

	/*
	 *
	 */
	struct SetPitchYaw: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			auto pitch = static_cast<float>(getNumericArg(stack, 1));
			auto yaw = static_cast<float>(getNumericArg(stack, 2));

			sgTransform->setPitchYaw(pitch, yaw);
		}
	};

	/*
	 *
	 */
	struct SetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			auto translation = getArg<Vec3>("vec3", stack, 1);

			sgTransform->setTranslation(translation);
		}
	};

	/*
	 *
	 */
	struct TranslateAbsolute: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			auto translation = getArg<Vec3>("vec3", stack, 1);

			sgTransform->translate(translation);
		}
	};

	/*
	 *
	 */
	struct TranslateRelative: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgTransform = std::static_pointer_cast<SgTransform>(self);

			auto translation = getArg<Vec3>("vec3", stack, 1);

			translation = sgTransform->getRotation().rotate(translation);

			sgTransform->translate(translation);
		}
	};

}

/**
 * constructor
 *
 * @param pitch
 * @param yaw
 * @param translation
 */
SgTransform::SgTransform(float p, float y, const Vec3 & t) :
		m_pitch(p), m_yaw(y), m_translation(t), m_rotation(p, y) {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr SgTransform::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getPitch", std::make_shared<GetPitch>() },
			{ "getRotation", std::make_shared<GetRotation>() },
			{ "getTranslation", std::make_shared<GetTranslation>() },
			{ "getYaw", std::make_shared<GetYaw>() },
			{ "setPitchYaw", std::make_shared<SetPitchYaw>() },
			{ "setTranslation", std::make_shared<SetTranslation>() },
			{ "translateAbsolute", std::make_shared<TranslateAbsolute>() },
			{ "translateRelative", std::make_shared<TranslateRelative>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 */
PRIVATE void SgTransform::recalculateRotation() {
	auto pi = static_cast<float>(M_PI);

	if (m_pitch > pi) {
		m_pitch = pi;
	}
	if (m_pitch < 0) {
		m_pitch = 0;
	}

	if (m_yaw > pi) {
		m_yaw -= 2 * pi;
	}
	if (m_yaw < -pi) {
		m_yaw += 2 * pi;
	}

	m_rotation.set(m_pitch, m_yaw);
}

/**
 *
 * @param builder
 */
OVERRIDE void SgTransform::taskInit(Builder & builder) {
	builder.translate(m_translation);
	builder.rotate(m_rotation);
}

/**
 *
 * @param state
 */
OVERRIDE void SgTransform::update(UpdateState & state) {
	state.translate(m_translation);
	state.rotate(m_rotation);
}

/**
 *
 * @param vb
 */
OVERRIDE void SgTransform::visualize(render::ViewBuilder & vb) {
	auto & state = vb.getState();
	state.translate(m_translation);
	state.rotate(m_rotation);
}

/**
 * get script object factory for SgTransform
 *
 * @return  SgTransform factory
 */
STATIC const ScriptObjectPtr & SgTransform::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
