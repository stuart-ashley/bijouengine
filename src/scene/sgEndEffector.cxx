#include "sgEndEffector.h"

#include "updateState.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("parent", nullptr),
			Parameter<Vec3>("pivotPos", nullptr),
			Parameter<Quat>("pivotRot", nullptr),
	};

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

			const auto & parent = std::static_pointer_cast<String>(
					args["parent"])->getValue();

			const auto & pivotPos = *std::static_pointer_cast<Vec3>(
					args["pivotPos"]);

			const auto & pivotRot = *std::static_pointer_cast<Quat>(
					args["pivotRot"]);

			stack.push(
					std::make_shared<SgEndEffector>(
							EndEffector(parent,
									Transform(pivotPos, pivotRot))));
		}
	};

	/*
	 *
	 */
	class SetConvergence: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgEndEffector = std::static_pointer_cast<SgEndEffector>(self);

			auto convergence = static_cast<float>(getNumericArg(stack, 1));

			sgEndEffector->setConvergence(convergence);
		}
	};

	/*
	 *
	 */
	class SetGoalRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgEndEffector = std::static_pointer_cast<SgEndEffector>(self);

			auto rotation = getArg<Quat>("Quat", stack, 1);

			sgEndEffector->setGoalRotation(rotation);
		}
	};

	/*
	 *
	 */
	class SetGoalTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgEndEffector = std::static_pointer_cast<SgEndEffector>(self);

			auto translation = getArg<Vec3>("Vec3", stack, 1);

			sgEndEffector->setGoalTranslation(translation);
		}
	};

}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgEndEffector::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "setConvergence", std::make_shared<SetConvergence>() },
			{ "setGoalRotation", std::make_shared<SetGoalRotation>() },
			{ "setGoalTranslation", std::make_shared<SetGoalTranslation>() }
	};

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @param state
 */
OVERRIDE void SgEndEffector::update(UpdateState & state) {
	state.addEndEffector(endEffector);
}

/**
 * get script object factory for SgEndEffector
 *
 * @return  SgEndEffector factory
 */
STATIC const ScriptObjectPtr & SgEndEffector::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
