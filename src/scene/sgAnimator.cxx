#include "sgAnimator.h"

#include "updateState.h"

#include "../core/animation.h"
#include "../core/bone.h"
#include "../core/config.h"
#include "../core/quat.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include "../scripting/executable.h"
#include "../scripting/kwarg.h"
#include "../scripting/none.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <cassert>

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto active = getArg<Animation>("Animation", stack, 1);
			stack.push(std::make_shared<SgAnimator>(active));
		}
	};

	/*
	 *
	 */
	class GetDeltaTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			auto dt = static_cast<float>(getNumericArg( stack, 2));

			stack.push(
					std::make_shared<Vec3>(
							sgAnimator->getDeltaTranslation(bone, dt)));
		}
	};

	/*
	 *
	 */
	class GetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			stack.push(std::make_shared<Quat>(sgAnimator->getRotation(bone)));
		}
	};

	/*
	 *
	 */
	class GetTransform: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			const auto & transforms = sgAnimator->getTransforms();

			const auto & it = transforms.find(bone);
			if (it != transforms.end()) {
				stack.push(std::make_shared<Transform>(it->second));
			} else {
				stack.push(None::none());
			}
		}
	};

	/*
	 *
	 */
	class GetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			stack.push(
					std::make_shared<Vec3>(sgAnimator->getTranslation(bone)));
		}
	};

	/*
	 *
	 */
	class RotZ: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			auto angle = static_cast<float>(getNumericArg(stack,2));

			Quat rot(Normal(0.f, 0.f, 1.f), angle);

			scriptExecutionAssert(sgAnimator->rotate(bone, rot),
					"Unknown bone '" + bone + "'");
		}
	};

	/*
	 *
	 */
	class SetAnimation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto animation = getArg<Animation>("Animation", stack, 1);

			sgAnimator->setAnimation(animation);
		}
	};

	/*
	 *
	 */
	class SetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			auto r = getArg<Quat>("Quat", stack, 2);

			sgAnimator->setRotation(bone, r);
		}
	};

	/*
	 *
	 */
	class SetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto sgAnimator = std::static_pointer_cast<SgAnimator>(self);

			auto bone = getArg<String>("string", stack, 1).getValue();

			auto t = getArg<Vec3>("Vec3", stack, 2);

			sgAnimator->setTranslation(bone, t);
		}
	};
}

struct SgAnimator::impl {

	Animation animation;
	std::unordered_map<std::string, Transform> transforms;
	float frame;

	impl(const Animation & active) :
			animation(active), frame(0) {
	}

	Transform getTransform(const std::string & name) {
		Transform transform;
		auto it = transforms.find(name);
		if (it != transforms.end()) {
			transform = it->second;
		}
		return transform;
	}

	Transform peek(const std::string & name, float step) {
		float speed = Config::getInstance().getFloat("simulationSpeed");
		float dt = speed * step;

		if (animation.validate() == false) {
			return Transform();
		}

		float frame2 = animation.nextFrame(frame, dt);
		float frameStep = animation.getFrameStep(dt);

		if (animation.hasBone(name) == false) {
			return transforms.at(name);
		}
		const auto & bone = animation.getBone(name);
		return bone.getTransform(getTransform(name), frame2, frameStep);
	}
};

/**
 * Constructor
 *
 * @param animations
 */
SgAnimator::SgAnimator(const Animation & active) :
		pimpl(new impl(active)) {
}

/**
 * destructor
 */
SgAnimator::~SgAnimator() {
}

/**
 *
 * @return
 */
Vec3 SgAnimator::getDeltaTranslation(const std::string & bone, float dt) const {
	Vec3 t;
	auto it = pimpl->transforms.find(bone);
	if (it != pimpl->transforms.end()) {
		t = pimpl->peek(bone, dt).getTranslation()
				- it->second.getTranslation();
	}
	return t;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgAnimator::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getDeltaTranslation", std::make_shared<GetDeltaTranslation>() },
			{ "getRotation", std::make_shared<GetRotation>() },
			{ "getTransform", std::make_shared<GetTransform>() },
			{ "getTranslation", std::make_shared<GetTranslation>() },
			{ "rotZ", std::make_shared<RotZ>() },
			{ "setAnimation", std::make_shared<SetAnimation>() },
			{ "setRotation", std::make_shared<SetRotation>() },
			{ "setTranslation", std::make_shared<SetTranslation>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * get rotation of named bone
 *
 * @param bone  name of bone
 *
 * @return      rotation for requested bone
 */
Quat SgAnimator::getRotation(const std::string & bone) const {
	Quat r(0, 0, 0, 1);
	const auto & it = pimpl->transforms.find(bone);
	if (it != pimpl->transforms.end()) {
		r = it->second.getRotation();
	}
	return r;
}

/**
 *
 * @return
 */
const std::unordered_map<std::string, Transform> & SgAnimator::getTransforms() const {
	return pimpl->transforms;
}

/**
 * get translation of named bone
 *
 * @param bone  name of bone
 *
 * @return      translation for requested bone
 */
Vec3 SgAnimator::getTranslation(const std::string & bone) const {
	Vec3 t;
	const auto & it = pimpl->transforms.find(bone);
	if (it != pimpl->transforms.end()) {
		t = it->second.getTranslation();
	}
	return t;
}

/**
 *
 * @param bone
 * @param rotation
 */
bool SgAnimator::rotate(const std::string & bone, const Quat & rotation) {
	const auto & it = pimpl->transforms.find(bone);
	if (it != pimpl->transforms.end()) {
		it->second.rotate(rotation);
		return true;
	}
	return false;
}

/**
 *
 * @param name
 * @return
 */
void SgAnimator::setAnimation(const Animation & animation) {
	pimpl->animation = animation;
}

/**
 *
 * @param bone
 * @param rotation
 */
void SgAnimator::setRotation(const std::string & bone, const Quat & rotation) {
	const auto & it = pimpl->transforms.find(bone);
	if (it != pimpl->transforms.end()) {
		it->second.setRotation(rotation);
	} else {
		pimpl->transforms.emplace(bone, Transform(Vec3(), rotation));
	}
}
/**
 *
 * @param bone
 * @param translation
 */
void SgAnimator::setTranslation(const std::string & bone,
		const Vec3 & translation) {
	const auto & it = pimpl->transforms.find(bone);
	if (it != pimpl->transforms.end()) {
		it->second.setTranslation(translation);
	} else {
		pimpl->transforms.emplace(bone,
				Transform(translation, Quat(0, 0, 0, 1)));
	}
}

/**
 *
 * @param state
 */
OVERRIDE void SgAnimator::update(UpdateState & state) {
	if (pimpl->animation.validate() == false) {
		return;
	}

	float speed = Config::getInstance().getFloat("simulationSpeed");
	float dt = speed * state.getTimeStep();

	pimpl->frame = pimpl->animation.nextFrame(pimpl->frame, dt);
	float frameStep = pimpl->animation.getFrameStep(dt);

	for (const auto & boneEntry : pimpl->animation.getBones()) {
		const auto & t = boneEntry.second.getTransform(
				pimpl->getTransform(boneEntry.first), pimpl->frame, frameStep);
		pimpl->transforms[boneEntry.first] = t;
	}

	state.setBoneTransforms(pimpl->transforms);
}

/**
 * get script object factory for SgAnimator
 *
 * @return  SgAnimator factory
 */
STATIC ScriptObjectPtr SgAnimator::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
