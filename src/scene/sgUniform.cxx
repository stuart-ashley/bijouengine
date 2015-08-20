#include "sgUniform.h"

#include "../core/mat3.h"
#include "../core/mat4.h"
#include "../core/vec3.h"

#include "../render/renderState.h"
#include "../render/texture.h"
#include "../render/uniform.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <array>
#include <memory>

using namespace render;

namespace {

	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {

			auto id = Uniform::getUID(
					getArg<String>("string", stack, 1).getValue());
			if (nArgs == 2) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(Mat3)) {
					auto m = std::static_pointer_cast<Mat3>(arg);
					stack.push(std::make_shared<SgUniform>(Uniform(id, *m)));
				} else if (typeid(*arg) == typeid(Mat4)) {
					auto m = std::static_pointer_cast<Mat4>(arg);
					stack.push(std::make_shared<SgUniform>(Uniform(id, *m)));
				} else if (typeid(*arg) == typeid(Texture)) {
					auto tex = std::static_pointer_cast<Texture>(arg);
					stack.push(std::make_shared<SgUniform>(Uniform(id, tex)));
				} else if (typeid(*arg) == typeid(Vec3)) {
					auto v = std::static_pointer_cast<Vec3>(arg);
					stack.push(std::make_shared<SgUniform>(Uniform(id, *v)));
				} else if (typeid(*arg) == typeid(Real)) {
					float x = std::static_pointer_cast<Real>(arg)->getFloat();
					stack.push(std::make_shared<SgUniform>(Uniform(id, x)));
				} else {
					scriptExecutionAssert(false,
							"Require Mat3, Mat4, Texture, Vec3 or float");
				}
			} else if (nArgs == 3) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				stack.push(std::make_shared<SgUniform>(Uniform(id, x, y)));
			} else if (nArgs == 4) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				float z = static_cast<float>(getNumericArg(stack, 4));
				stack.push(std::make_shared<SgUniform>(Uniform(id, x, y, z)));
			} else if (nArgs == 5) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				float z = static_cast<float>(getNumericArg(stack, 4));
				float w = static_cast<float>(getNumericArg(stack, 5));
				stack.push(
						std::make_shared<SgUniform>(Uniform(id, x, y, z, w)));
			} else if (nArgs == 28) {
				std::array<float, 27> coefficients;
				for (int i = 0; i < 27; ++i) {
					coefficients[i] = static_cast<float>(getNumericArg(stack,
							i + 2));
				}
				stack.push(
						std::make_shared<SgUniform>(Uniform(id, coefficients)));
			} else {
				scriptExecutionAssert(false, "Unsupported number of arguments");
			}

		}
	};

	struct Set: public Executable {

		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			scriptExecutionAssert(nArgs >= 1, "Require at least 1 arguments");

			auto uniform = std::static_pointer_cast<SgUniform>(self);
			auto id = uniform->getId();

			if (nArgs == 1) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(Mat3)) {
					auto m = std::static_pointer_cast<Mat3>(arg);
					uniform->set(Uniform(id, *m));
				} else if (typeid(*arg) == typeid(Mat4)) {
					auto m = std::static_pointer_cast<Mat4>(arg);
					uniform->set(Uniform(id, *m));
				} else if (typeid(*arg) == typeid(Texture)) {
					auto tex = std::static_pointer_cast<Texture>(arg);
					uniform->set(Uniform(id, tex));
				} else if (typeid(*arg) == typeid(Vec3)) {
					auto v = std::static_pointer_cast<Vec3>(arg);
					uniform->set(Uniform(id, *v));
				} else if (typeid(*arg) == typeid(Real)) {
					float x = std::static_pointer_cast<Real>(arg)->getFloat();
					uniform->set(Uniform(id, x));
				} else {
					scriptExecutionAssert(false,
							"Require Mat3, Mat4, Texture, Vec3 or float");
				}
			} else if (nArgs == 3) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				uniform->set(Uniform(id, x, y));
			} else if (nArgs == 4) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				float z = static_cast<float>(getNumericArg(stack, 4));
				uniform->set(Uniform(id, x, y, z));
			} else if (nArgs == 5) {
				float x = static_cast<float>(getNumericArg(stack, 2));
				float y = static_cast<float>(getNumericArg(stack, 3));
				float z = static_cast<float>(getNumericArg(stack, 4));
				float w = static_cast<float>(getNumericArg(stack, 5));
				uniform->set(Uniform(id, x, y, z, w));
			} else if (nArgs == 28) {
				std::array<float, 27> coefficients;
				for (int i = 0; i < 27; ++i) {
					coefficients[i] = static_cast<float>(getNumericArg(stack,
							i + 2));
				}
				uniform->set(Uniform(id, coefficients));
			} else {
				scriptExecutionAssert(false, "Unsupported number of arguments");
			}
		}
	};
}

/**
 * Construct from uniform
 *
 * @param uniform
 *            Uniform to wrap
 */
SgUniform::SgUniform(const render::Uniform & uniform) :
		m_uniform(uniform) {
}

/**
 * get uniform id
 *
 * @return  uniform id
 */
size_t SgUniform::getId() const {
	return m_uniform.getUID();
}

/**
 * set uniform value
 *
 * @param uniform  uniform value
 */
void SgUniform::set(const render::Uniform & uniform) {
	m_uniform = uniform;
}

/**
 * Add uniform to render state
 */
OVERRIDE void SgUniform::visualize(render::ViewBuilder & vb) {
	vb.getState().addUniform(m_uniform);
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgUniform::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = { { "set",
			std::make_shared<Set>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * get script object factory for SgUniform
 *
 * @return  SgUniform factory
 */
STATIC const ScriptObjectPtr & SgUniform::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
