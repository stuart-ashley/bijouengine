#define _USE_MATH_DEFINES
#include "bone.h"

#include "bezier.h"
#include "binary.h"
#include "quat.h"
#include "transform.h"
#include "vec3.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <cassert>
#include <cmath>

namespace {

	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			scriptExecutionAssert(nArgs >= 2, "Require at least 2 arguments");

			auto name = getArg<String>("string", stack, 1).getValue();

			std::vector<Bezier> beziers;
			for (unsigned i = 1; i < nArgs; ++i) {
				auto arg = stack.top();
				stack.pop();

				scriptExecutionAssertType<Binary>(arg,
						"Require Bezier for argument " + i);

				beziers.emplace_back(*std::static_pointer_cast<Binary>(arg));
			}
			stack.push(std::make_shared<Bone>(name, beziers));
		}
	};
}

struct Bone::impl {
	std::string name;
	std::vector<Bezier> curves;
	bool valid;

	impl(const std::string & name, const std::vector<Bezier> & curves) :
			name(name), curves(curves), valid(false) {
	}

};

/**
 * constructor
 *
 * @param name    name of bone
 * @param curves  list of curves for bone
 */
Bone::Bone(const std::string & name, const std::vector<Bezier> & curves) :
		pimpl(new impl(name, curves)) {
}

/**
 * destructor
 */
Bone::~Bone() {
}

/**
 * get bone name
 *
 * @return  name of bone
 */
const std::string & Bone::getName() {
	return pimpl->name;
}

/**
 * get curve at given index
 *
 * @param idx  index to get
 *
 * @return     curve at index
 */
const Bezier & Bone::get(int idx) const {
	return pimpl->curves.at(idx);
}

/**
 * get number of curves
 *
 * @return  number of curves
 */
size_t Bone::size() const {
	return pimpl->curves.size();
}

/**
 *
 * @param current  current transform
 * @param frame    current frame
 * @param d        frame step
 *
 * @return         updated transform
 */
Transform Bone::getTransform(const Transform & current, float frame,
		float d) const {
	Vec3 translation = current.getTranslation();
	Quat rotation = current.getRotation();

	float rx = 0, ry = 0, rz = 0, rw = 1;
	double tx = 0, ty = 0, tz = 0;

	int flags = 0;

	for (const auto & curve : pimpl->curves) {
		if (curve.getName() == "LocX") {
			assert((flags & 0x0381) == 0); // 0000 0011 1000 0001
			tx = curve.getY(frame);
			flags |= 0x0001; // 0000 0000 0000 0001
			continue;
		}
		if (curve.getName() == "LocY") {
			assert((flags & 0x0382) == 0); // 0000 0011 1000 0010
			ty = curve.getY(frame);
			flags |= 0x0002; // 0000 0000 0000 0010
			continue;
		}
		if (curve.getName() == "LocZ") {
			assert((flags & 0x0384) == 0); // 0000 0011 1000 0100
			tz = curve.getY(frame);
			flags |= 0x0004; // 0000 0000 0000 0100
			continue;
		}
		if (curve.getName() == "QuatX") {
			assert((flags & 0x1c08) == 0); // 0001 1100 0000 1000
			rx = curve.getY(frame);
			flags |= 0x0008; // 0000 0000 0000 1000
			continue;
		}
		if (curve.getName() == "QuatY") {
			assert((flags & 0x1c10) == 0); // 0001 1100 0001 0000
			ry = curve.getY(frame);
			flags |= 0x0010; // 0000 0000 0001 0000
			continue;
		}
		if (curve.getName() == "QuatZ") {
			assert((flags & 0x1c20) == 0); // 0001 1100 0010 0000
			rz = curve.getY(frame);
			flags |= 0x0020; // 0000 0000 0010 0000
			continue;
		}
		if (curve.getName() == "QuatW") {
			assert((flags & 0x1c40) == 0); // 0001 1100 0100 0000
			rw = curve.getY(frame);
			flags |= 0x0040; // 0000 0000 0100 0000
			continue;
		}
		if (curve.getName() == "dLocX") {
			assert((flags & 0x0087) == 0); // 0000 0000 1000 0111
			double x = curve.getY(frame) * d;
			translation += rotation.rotateX(x);
			flags |= 0x0080; // 0000 0000 1000 0000
			continue;
		}
		if (curve.getName() == "dLocY") {
			assert((flags & 0x0107) == 0); // 0000 0001 0000 0111
			double y = curve.getY(frame) * d;
			translation += rotation.rotateY(y);
			flags |= 0x0100; // 0000 0001 0000 0000
			continue;
		}
		if (curve.getName() == "dLocZ") {
			assert((flags & 0x0207) == 0); // 0000 0010 0000 0111
			double z = curve.getY(frame) * d;
			translation += rotation.rotateZ(z);
			flags |= 0x0200; // 0000 0010 0000 0000
			continue;
		}
		if (curve.getName() == "dRotX") {
			assert((flags & 0x0478) == 0); // 0000 0100 0111 1000
			float angle = static_cast<float>(curve.getY(frame) / 180 * M_PI * d);
			rotation *= Quat(Normal(1.f, 0.f, 0.f), angle);
			flags |= 0x0400; // 0000 0100 0000 0000
			continue;
		}
		if (curve.getName() == "dRotY") {
			assert((flags & 0x0878) == 0); // 0000 1000 0111 1000
			float angle = static_cast<float>(curve.getY(frame) / 180 * M_PI * d);
			rotation *= Quat(Normal(0.f, 1.f, 0.f), angle);
			flags |= 0x0800; // 0000 1000 0000 0000
			continue;
		}
		if (curve.getName() == "dRotZ") {
			assert((flags & 0x1078) == 0); // 0001 0000 0111 1000
			float angle = static_cast<float>(curve.getY(frame) / 180 * M_PI * d);
			rotation *= Quat(Normal(0.f, 0.f, 1.f), angle);
			flags |= 0x1000; // 0001 0000 0000 0000
			continue;
		}
	}

	if ((flags & 0x0078) > 0) { // 0000 0000 0111 1000
		assert((flags & 0x0078) == 0x0078);
		rotation.set(rx, ry, rz, rw);
	}

	if ((flags & 0x0007) > 0) { // 0000 0000 0000 0111
		assert((flags & 0x0007) == 0x0007);
		translation.set(tx, ty, tz);
	}
	return Transform(translation, rotation);
}

/**
 * validate bone (ie check loaded)
 *
 * @return  true if valid, false otherwise
 */
bool Bone::validate() const {
	if (pimpl->valid) {
		return true;
	}
	pimpl->valid = true;
	for (const auto & curve : pimpl->curves) {
		pimpl->valid = pimpl->valid && curve.validate();
	}
	return pimpl->valid;
}

/**
 * get script object factory for Bone
 *
 * @return  Bone factory
 */
STATIC const ScriptObjectPtr & Bone::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
