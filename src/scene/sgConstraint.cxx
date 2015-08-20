#include "sgConstraint.h"

#include "updateState.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/string.h"

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("body0", nullptr),
			Parameter<Vec3>("pivot0Pos", nullptr),
			Parameter<Quat>("pivot0Rot", nullptr),
			Parameter<String>("body1", nullptr),
			Parameter<Vec3>("pivot1Pos", nullptr),
			Parameter<Quat>("pivot1Rot", nullptr),
			Parameter<Real>("limitFlags", nullptr),
			Parameter<Vec3>("minPos", std::make_shared<Vec3>()),
			Parameter<Vec3>("maxPos", std::make_shared<Vec3>()),
			Parameter<Vec3>("minRot", std::make_shared<Vec3>()),
			Parameter<Vec3>("maxRot", std::make_shared<Vec3>()),
			Parameter<Real>("springyness", std::make_shared<Real>(0)),
			Parameter<Real>("stiffness", std::make_shared<Real>(0)) };

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

			auto body0 =
					std::static_pointer_cast<String>(args["body0"])->getValue();

			auto pivot0Pos = *std::static_pointer_cast<Vec3>(args["pivot0Pos"]);

			auto pivot0Rot = *std::static_pointer_cast<Quat>(args["pivot0Rot"]);

			auto body1 =
					std::static_pointer_cast<String>(args["body1"])->getValue();

			auto pivot1Pos = *std::static_pointer_cast<Vec3>(args["pivot1Pos"]);

			auto pivot1Rot = *std::static_pointer_cast<Quat>(args["pivot1Rot"]);

			int limitFlags =
					std::static_pointer_cast<Real>(args["limitFlags"])->getInt32();

			auto minPos = *std::static_pointer_cast<Vec3>(args["minPos"]);

			auto maxPos = *std::static_pointer_cast<Vec3>(args["maxPos"]);

			auto minRot = *std::static_pointer_cast<Vec3>(args["minRot"]);

			auto maxRot = *std::static_pointer_cast<Vec3>(args["maxRot"]);

			float springyness = std::static_pointer_cast<Real>(
					args["springyness"])->getFloat();

			float stiffness =
					std::static_pointer_cast<Real>(args["stiffness"])->getFloat();

			stack.push(
					std::make_shared<SgConstraint>(
							Constraint(body0, pivot0Pos, pivot0Rot, body1,
									pivot1Pos, pivot1Rot, limitFlags, minPos,
									maxPos, minRot, maxRot, springyness,
									stiffness)));
		}
	};

	/*
	 *
	 */
	struct GetPivot0Pos: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			stack.push(std::make_shared<Vec3>(constraint.getPivot0Pos()));
		}
	};

	/*
	 *
	 */
	struct SetBody0: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			auto name = getArg<String>("string", stack, 1).getValue();

			constraint.setBodyName0(name);
		}
	};

	/*
	 *
	 */
	struct SetBody1: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			auto name = getArg<String>("string", stack, 1).getValue();

			constraint.setBodyName1(name);
		}
	};

	/*
	 *
	 */
	struct SetPivot0Rot: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			auto rot = getArg<Quat>("Quat", stack, 1);

			constraint.setPivot0Rot(rot);
		}
	};

	/*
	 *
	 */
	struct SetPivot0Pos: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			auto pos = getArg<Vec3>("Vec3", stack, 1);

			constraint.setPivot0Pos(pos);
		}
	};

	/*
	 *
	 */
	struct SetPivot1Rot: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			auto rot = getArg<Quat>("Quat", stack, 1);

			constraint.setPivot1Rot(rot);
		}
	};

	/*
	 *
	 */
	struct SetPivot1Pos: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & constraint =
					std::static_pointer_cast<SgConstraint>(self)->getConstraint();

			auto pos = getArg<Vec3>("Vec3", stack, 1);

			constraint.setPivot1Pos(pos);
		}
	};
}

/**
 * Construct from constraint
 *
 * @param constraint
 */
SgConstraint::SgConstraint(const Constraint & constraint) :
		constraint(constraint) {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgConstraint::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getPivot0Pos", std::make_shared<GetPivot0Pos>() },
			{ "setBody0", std::make_shared<SetBody0>() },
			{ "setBody1", std::make_shared<SetBody1>() },
			{ "setPivot0Rot", std::make_shared<SetPivot0Rot>() },
			{ "setPivot0Pos", std::make_shared<SetPivot0Pos>() },
			{ "setPivot1Rot", std::make_shared<SetPivot1Rot>() },
			{ "setPivot1Pos", std::make_shared<SetPivot1Pos>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * On update add constraint to state
 */
OVERRIDE void SgConstraint::update(UpdateState & state) {
	if (constraint.getBodyName0() != "" && constraint.getBodyName1() != "") {
		state.addConstraint(constraint);
	}
}

/**
 * get script object factory for SgConstraint
 *
 * @return  SgConstraint factory
 */
STATIC const ScriptObjectPtr & SgConstraint::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
