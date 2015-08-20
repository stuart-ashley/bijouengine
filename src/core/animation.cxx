#include "animation.h"

#include "bezier.h"
#include "binary.h"
#include "bone.h"

#include "../scripting/executable.h"
#include "../scripting/kwarg.h"
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
			scriptExecutionAssert(nArgs >= 5, "Require at least 5 arguments");

			int flags = 0x00; // 0000 0000

			std::string name;
			float sFrame;
			float eFrame;
			float offset = 0;
			float fps;
			std::vector<Bezier> beziers;
			std::unordered_map<std::string, Bone> bones;
			for (unsigned i = 0; i < nArgs; ++i) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(Kwarg)) {
					auto key = std::static_pointer_cast<Kwarg>(arg)->getKey();
					if (key == "name") {
						// 0000 0001
						scriptExecutionAssert((flags & 0x01) == 0,
								"Duplicate name");
						name = getArg<String>("string", stack, 1).getValue();
						flags |= 0x01;
					} else if (key == "sFrame") {
						// 0000 0010
						scriptExecutionAssert((flags & 0x02) == 0,
								"Duplicate sFrame");
						sFrame = static_cast<float>(getNumericArg(stack, 1));
						flags |= 0x02;
					} else if (key == "eFrame") {
						// 0000 0100
						scriptExecutionAssert((flags & 0x04) == 0,
								"Duplicate eFrame");
						eFrame = static_cast<float>(getNumericArg(stack, 1));
						flags |= 0x04;
					} else if (key == "offset") {
						// 0000 1000
						scriptExecutionAssert((flags & 0x08) == 0,
								"Duplicate offset");
						offset = static_cast<float>(getNumericArg(stack, 1));
						flags |= 0x08;
					} else if (key == "fps") {
						// 0001 0000
						scriptExecutionAssert((flags & 0x10) == 0,
								"Duplicate fps");
						fps = static_cast<float>(getNumericArg(stack, 1));
						flags |= 0x10;
					} else {
						scriptExecutionAssert(false,
								"Require name, sFrame, eFrame, offset or fps got: '"
										+ key + "'");
					}
				} else if (typeid(*arg) == typeid(Binary)) {
					beziers.emplace_back(
							*std::static_pointer_cast<Binary>(arg));
				} else if (typeid(*arg) == typeid(Bone)) {
					auto bone = *std::static_pointer_cast<Bone>(arg);
					bones.emplace(bone.getName(), bone);
				} else {
					scriptExecutionAssert(false,
							"Require keyword, Binary or Bone for argument "
									+ i);
				}
			}
			// 0001 0111
			scriptExecutionAssert((flags & 0x17) == 0x17,
					"Require name, sFrame, eFrame, and fps");
			if (beziers.size() > 0) {
				bones.emplace("", Bone("", beziers));
			}
			stack.push(
					std::make_shared<Animation>(name, sFrame, eFrame, offset,
							fps, bones));
		}
	};
}

struct Animation::impl {

	std::string name;
	float sFrame;
	float eFrame;
	float offset;
	float fps;
	std::unordered_map<std::string, Bone> bones;
	bool valid = false;

	impl(const std::string & name, float sFrame, float eFrame, float offset,
			float fps, const std::unordered_map<std::string, Bone> & bones) :
					name(name),
					sFrame(sFrame),
					eFrame(eFrame),
					offset(offset),
					fps(fps),
					bones(bones),
					valid(false) {
	}
};

/**
 * constructor
 *
 * @param name
 * @param sFrame
 * @param eFrame
 * @param offset
 * @param fps
 * @param bones
 */
Animation::Animation(const std::string & name, float sFrame, float eFrame,
		float offset, float fps,
		const std::unordered_map<std::string, Bone> & bones) :
		pimpl(new impl(name, sFrame, eFrame, offset, fps, bones)) {
}

/**
 * destructor
 */
Animation::~Animation() {
}

/**
 * Get first animation element
 *
 * @return first animation element
 */
const Bone & Animation::getBone() const {
	assert(pimpl->bones.size() == 1);
	return pimpl->bones.cbegin()->second;
}

/**
 * get named bone
 *
 * @param name  name of bone to get
 *
 * @return      named bone
 */
const Bone & Animation::getBone(const std::string & name) const {
	return pimpl->bones.at(name);
}

/**
 * Get animation elements
 *
 * @return list of animation elements
 */
const std::unordered_map<std::string, Bone> & Animation::getBones() const {
	return pimpl->bones;
}

/**
 * get end frame of animation
 *
 * @return  end frame
 */
float Animation::getEndFrame() const {
	return pimpl->eFrame;
}

/**
 * get animation speed in frames per second
 *
 * @return  frames per second
 */
float Animation::getFPS() const {
	return pimpl->fps;
}

/**
 * get frame step from delta time
 *
 * @param dt  delta time in seconds
 *
 * @return    frame step
 */
float Animation::getFrameStep(float dt) const {
	return pimpl->fps * dt;
}

/**
 * get animation offset
 *
 * @return  offset of animation
 */
float Animation::getOffset() const {
	return pimpl->offset;
}

/**
 * get animation name
 *
 * @return  name of animation
 */
const std::string & Animation::getName() const {
	return pimpl->name;
}

/**
 * get start frame of animation
 *
 * @return  start frame
 */
float Animation::getStartFrame() const {
	return pimpl->sFrame;
}

/**
 * check existence of named bone
 *
 * @param name  name of bone to check
 *
 * @return      true if bone exists, false otherwise
 */
bool Animation::hasBone(const std::string & name) const {
	return pimpl->bones.find(name) != pimpl->bones.end();
}

/**
 * get length of animation
 *
 * @return  length of animation
 */
float Animation::length() const {
	return (pimpl->eFrame - pimpl->sFrame + 1) / pimpl->fps;
}

/**
 * calculate next animation frame, wrap as necessary
 *
 * @param frame  current animation frame
 * @param dt     delta time in seconds
 *
 * @return       next animation frame
 */
float Animation::nextFrame(float frame, float dt) const {
	float frame2 = frame + getFrameStep(dt);
	while (frame2 < pimpl->sFrame) {
		frame2 += pimpl->eFrame - pimpl->sFrame;
	}
	while (frame2 > pimpl->eFrame) {
		frame2 -= pimpl->eFrame - pimpl->sFrame;
	}
	return frame2;
}

/**
 * Validate elements
 *
 * @return true if all elements valid, false otherwise
 */
bool Animation::validate() const {
	if (pimpl->valid == true) {
		return true;
	}
	pimpl->valid = true;
	for (const auto & entry : pimpl->bones) {
		pimpl->valid = pimpl->valid && entry.second.validate();
	}
	return pimpl->valid;
}

/**
 * get script object factory for Animation
 *
 * @return  Animation factory
 */
STATIC const ScriptObjectPtr & Animation::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
