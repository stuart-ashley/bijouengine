#include "sgRigidBody.h"

#include "builder.h"
#include "sgNode.h"
#include "updateState.h"

#include "../core/boundingBox.h"
#include "../core/collisionHierarchy.h"
#include "../core/debugGeometry.h"
#include "../core/rigidBody.h"
#include "../core/transform.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/none.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/string.h"

#include <cassert>
#include <cmath>

using namespace render;

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("name", nullptr),
			Parameter<Real>("inverseMass", nullptr),
			Parameter<Vec3>("translation", nullptr),
			Parameter<Quat>("rotation", nullptr),
			Parameter<Vec3>("velocity", std::make_shared<Vec3>()),
			Parameter<Vec3>("angularVelocity", std::make_shared<Vec3>()),
			Parameter<Vec3>("gravity", std::make_shared<Vec3>(0, 0, -9.8)),
			Parameter<CollisionHierarchy>("collision", nullptr),
			Parameter<List>("nocollide", std::make_shared<List>()),
			Parameter<Bool>("friction", Bool::False()),
			Parameter<SgNode>("model", None::none()) };

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

			auto name =
					std::static_pointer_cast<String>(args["name"])->getValue();

			float inverseMass = std::static_pointer_cast<Real>(
					args["inverseMass"])->getFloat();

			auto translation = std::static_pointer_cast<Vec3>(
					args["translation"]);

			auto rotation = std::static_pointer_cast<Quat>(args["rotation"]);

			auto velocity = std::static_pointer_cast<Vec3>(args["velocity"]);

			auto angularVelocity = std::static_pointer_cast<Vec3>(
					args["angularVelocity"]);

			auto gravity = std::static_pointer_cast<Vec3>(args["gravity"]);

			auto collision = std::static_pointer_cast<CollisionHierarchy>(
					args["collision"]);

			std::unordered_set<std::string> nocollide;
			auto list = std::static_pointer_cast<List>(args["nocollide"]);
			for (const auto & e : *list) {
				scriptExecutionAssertType<String>(e, "Require string list");

				nocollide.emplace(
						std::static_pointer_cast<String>(e)->getValue());
			}

			bool doFriction = std::static_pointer_cast<Bool>(args["friction"])
					== Bool::True();

			auto model = args["model"];

			double sgp = 0;

			Transform rotTrans(*translation, *rotation);

			RigidBody body(name, inverseMass, rotTrans, *velocity,
					*angularVelocity, *collision, *gravity, sgp, nocollide,
					doFriction);

			stack.push(std::make_shared<SgRigidBody>(body, model));
		}
	};

	/*
	 *
	 */
	class Save: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			// No need to save static bodies
			if (rigidBody.getInverseMass() == 0) {
				return;
			}

			stack.push(std::make_shared<String>("translation"));
			stack.push(std::make_shared<Vec3>(rigidBody.getTranslation()));
			stack.push(std::make_shared<String>("rotation"));
			stack.push(std::make_shared<Quat>(rigidBody.getRotation()));
			stack.push(std::make_shared<String>("velocity"));
			stack.push(std::make_shared<Vec3>(rigidBody.getLinearVelocity()));
			stack.push(std::make_shared<String>("angularVelocity"));
			stack.push(std::make_shared<Vec3>(rigidBody.getAngularVelocity()));
		}
	};

	/*
	 *
	 */
	class Restore: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			assert(getArg<String>("string", stack, 1).getValue() == "translation");

			rigidBody.setTranslation(getArg<Vec3>("vec3", stack, 2));

			assert(getArg<String>("string", stack, 3).getValue() == "rotation");

			rigidBody.setRotation(getArg<Quat>("quat", stack, 4));

			assert(getArg<String>("string", stack, 5).getValue() == "velocity");

			rigidBody.setLinearVelocity(getArg<Vec3>("vec3", stack, 6));

			assert(getArg<String>("string", stack, 7).getValue() == "angularVelocity");

			rigidBody.setAngularVelocity(getArg<Vec3>("vec3", stack, 8));
		}
	};

	/*
	 *
	 */
	class GetYaw: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			// XXX rotation about z ( problem if up axis not z )
			Quat r = rigidBody.getRotation();

			float yaw = (float) std::atan2(
					2 * (r.getX() * r.getY() + r.getW() * r.getZ()),
					1 - 2 * (r.getY() * r.getY() + r.getZ() * r.getZ()));
			stack.push(std::make_shared<Real>(yaw));
		}
	};

	/*
	 *
	 */
	class GetLoaded: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			const auto & sgRigidBody = std::static_pointer_cast<SgRigidBody>(
					self);

			if (sgRigidBody->valid()) {
				stack.push(Bool::True());
			} else {
				stack.push(Bool::False());
			}
		}
	};

	/*
	 *
	 */
	class GetTransform: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<Transform>(rigidBody.getTransform()));
		}
	};

	/*
	 *
	 */
	class SetTransform: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgRigidBody = std::static_pointer_cast<SgRigidBody>(self);

			auto transform = getArg<Transform>("transform", stack, 1);

			sgRigidBody->setTranslation(transform.getTranslation());
			sgRigidBody->setRotation(transform.getRotation());
		}
	};

	/*
	 *
	 */
	class GetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<Vec3>(rigidBody.getTranslation()));
		}
	};

	/*
	 *
	 */
	class SetTranslation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgRigidBody = std::static_pointer_cast<SgRigidBody>(self);

			sgRigidBody->setTranslation(getArg<Vec3>("vec3", stack, 1));
		}
	};

	/*
	 *
	 */
	class GetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<Quat>(rigidBody.getRotation()));
		}
	};

	/*
	 *
	 */
	class SetRotation: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgRigidBody = std::static_pointer_cast<SgRigidBody>(self);

			sgRigidBody->setRotation(getArg<Quat>("quat", stack, 1));
		}
	};

	/*
	 *
	 */
	class GetVelocity: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<Vec3>(rigidBody.getLinearVelocity()));
		}
	};

	/*
	 *
	 */
	class SetVelocity: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			auto velocity = getArg<Vec3>("vec3", stack, 1);
			rigidBody.setLinearVelocity(velocity);
		}
	};

	/*
	 *
	 */
	class GetAngularVelocity: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<Vec3>(rigidBody.getAngularVelocity()));
		}
	};

	class SetAngularVelocity: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			auto angularVelocity = getArg<Vec3>("vec3", stack, 1);
			rigidBody.setAngularVelocity(angularVelocity);
		}
	};

	/*
	 *
	 */
	class GetBounds: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(
					std::make_shared<BoundingBox>(
							rigidBody.getCollision().getBounds()));
		}
	};

	/*
	 *
	 */
	class GetSgp: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<Real>(rigidBody.getSgp()));
		}
	};

	/*
	 *
	 */
	class AddVelocity: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			auto v = getArg<Vec3>("vec3", stack, 1);

			rigidBody.setLinearVelocity(rigidBody.getLinearVelocity() + v);
		}
	};

	/*
	 *
	 */
	class AddRelativeVelocity: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			auto rotation = rigidBody.getRotation();

			rigidBody.setLinearVelocity(
					rigidBody.getLinearVelocity()
							+ rotation.rotate(getArg<Vec3>("vec3", stack, 1)));
		}
	};

	/*
	 *
	 */
	class ApplyImpulse: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			auto point = getArg<Vec3>("vec3", stack, 1);
			auto impulse = getArg<Vec3>("vec3", stack, 1);

			rigidBody.applyImpulse(point, impulse);
		}
	};

	/*
	 *
	 */
	class Freeze: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> &) const override {
			checkNumArgs(nArgs, 0);

			std::static_pointer_cast<SgRigidBody>(self)->freeze();
		}
	};

	/*
	 *
	 */
	class Unfreeze: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> &) const override {
			checkNumArgs(nArgs, 0);

			std::static_pointer_cast<SgRigidBody>(self)->unfreeze();
		}
	};

	/*
	 *
	 */
	class GetModel: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto sgRigidBody = std::static_pointer_cast<SgRigidBody>(self);

			stack.push(sgRigidBody->getModel());
		}
	};

	/*
	 *
	 */
	class SetModel: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto sgRigidBody = std::static_pointer_cast<SgRigidBody>(self);

			sgRigidBody->setModel(stack.top());
			stack.pop();
		}
	};

	/*
	 *
	 */
	class GetName: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rigidBody =
					std::static_pointer_cast<SgRigidBody>(self)->getRigidBody();

			stack.push(std::make_shared<String>(rigidBody.getName()));
		}
	};
}

struct SgRigidBody::impl {

	RigidBody rigidBody;
	ScriptObjectPtr model;
	Transform renderTransform;

	std::shared_ptr<UpdateNode> modelAsUpdateNode;
	std::shared_ptr<TaskInitNode> modelAsTaskInitNode;
	std::shared_ptr<VisualizeNode> modelAsVisualizeNode;

	bool valid;
	bool freeze;

	Vec3 newTranslation;
	Quat newRotation;
	ScriptObjectPtr newModel;
	bool gotNewTranslation;
	bool gotNewRotation;

	Vec3 initialTranslation;
	Quat initialRotation;
	Vec3 initialVelocity;
	Vec3 initialAngularVelocity;

	impl(const RigidBody & rb, const ScriptObjectPtr & model) :
					rigidBody(rb),
					model(model),
					valid(false),
					freeze(false),
					newModel(model),
					gotNewTranslation(false),
					gotNewRotation(false),
					initialTranslation(rb.getTranslation()),
					initialRotation(rb.getRotation()),
					initialVelocity(rb.getLinearVelocity()),
					initialAngularVelocity(rb.getAngularVelocity()) {
	}
};

/**
 * constructor
 *
 * @param rb
 * @param model
 */
SgRigidBody::SgRigidBody(const RigidBody & rb, const ScriptObjectPtr & model) :
		pimpl(new impl(rb, model)) {
}

/**
 * destructor
 */
SgRigidBody::~SgRigidBody() {
}

/**
 *
 */
void SgRigidBody::freeze() {
	pimpl->freeze = true;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgRigidBody::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "addRelativeVelocity", std::make_shared<AddRelativeVelocity>() },
			{ "addVelocity", std::make_shared<AddVelocity>() },
			{ "applyImpulse", std::make_shared<ApplyImpulse>() },
			{ "freeze", std::make_shared<Freeze>() },
			{ "getAngularVelocity", std::make_shared<GetAngularVelocity>() },
			{ "getBounds", std::make_shared<GetBounds>() },
			{ "getLoaded", std::make_shared<GetLoaded>() },
			{ "getName", std::make_shared<GetName>() },
			{ "getModel", std::make_shared<GetModel>() },
			{ "getRotation", std::make_shared<GetRotation>() },
			{ "getSgp", std::make_shared<GetSgp>() },
			{ "getTransform", std::make_shared<GetTransform>() },
			{ "getTranslation", std::make_shared<GetTranslation>() },
			{ "getVelocity", std::make_shared<GetVelocity>() },
			{ "getYaw", std::make_shared<GetYaw>() },
			{ "restore", std::make_shared<Restore>() },
			{ "save", std::make_shared<Save>() },
			{ "setAngularVelocity", std::make_shared<SetAngularVelocity>() },
			{ "setModel", std::make_shared<SetModel>() },
			{ "setRotation", std::make_shared<SetRotation>() },
			{ "setTransform", std::make_shared<SetTransform>() },
			{ "setTranslation", std::make_shared<SetTranslation>() },
			{ "setVelocity", std::make_shared<SetVelocity>() },
			{ "unfreeze", std::make_shared<Unfreeze>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @return
 */
RigidBody & SgRigidBody::getRigidBody() const {
	return pimpl->rigidBody;
}

/**
 *
 * @return
 */
const ScriptObjectPtr & SgRigidBody::getModel() const {
	return pimpl->model;
}

/**
 *
 * @param model
 */
void SgRigidBody::setModel(const ScriptObjectPtr & model) const {
	pimpl->newModel = model;
}

/**
 *
 * @param rotation
 */
void SgRigidBody::setRotation(const Quat & rotation) const {
	pimpl->newRotation = rotation;
	pimpl->gotNewRotation = true;
}

/**
 *
 * @param translation
 */
void SgRigidBody::setTranslation(const Vec3 & translation) const {
	pimpl->newTranslation = translation;
	pimpl->gotNewTranslation = true;
}

/**
 *
 * @param builder
 */
OVERRIDE void SgRigidBody::taskInit(Builder & builder) {
	if (pimpl->valid == false) {
		return;
	}

	// do transform
	pimpl->renderTransform = pimpl->rigidBody.getTransform().to(
			builder.getTransform());

	if (pimpl->modelAsTaskInitNode != nullptr) {
		builder.pushState();
		builder.transform(pimpl->renderTransform);

		// model
		pimpl->modelAsTaskInitNode->taskInit(builder);

		builder.popState();
	}
}

/**
 *
 */
void SgRigidBody::unfreeze() {
	pimpl->freeze = false;
}

/**
 *
 * @param state
 */
OVERRIDE void SgRigidBody::update(UpdateState & state) {
	pimpl->valid = pimpl->rigidBody.validate();
	if (pimpl->valid == false) {
		return;
	}

	if (pimpl->gotNewTranslation) {
		pimpl->rigidBody.setTranslation(pimpl->newTranslation);
		pimpl->gotNewTranslation = false;
	}
	if (pimpl->gotNewRotation) {
		pimpl->rigidBody.setRotation(pimpl->newRotation);
		pimpl->gotNewRotation = false;
	}
	if (pimpl->newModel != nullptr) {
		pimpl->model = pimpl->newModel;
		pimpl->newModel = nullptr;

		pimpl->modelAsUpdateNode = std::dynamic_pointer_cast<UpdateNode>(pimpl->model);
		pimpl->modelAsTaskInitNode = std::dynamic_pointer_cast<TaskInitNode>(pimpl->model);
		pimpl->modelAsVisualizeNode = std::dynamic_pointer_cast<VisualizeNode>(pimpl->model);
	}

	if (state.getEvents("reset").size() != 0) {
		pimpl->rigidBody.setTranslation(pimpl->initialTranslation);
		pimpl->rigidBody.setRotation(pimpl->initialRotation);
		pimpl->rigidBody.setLinearVelocity(pimpl->initialVelocity);
		pimpl->rigidBody.setAngularVelocity(pimpl->initialAngularVelocity);
	}

	// add body
	if (pimpl->freeze == false) {
		state.addRigidBody(pimpl->rigidBody);
	}

	if (pimpl->modelAsUpdateNode != nullptr) {
		state.pushState();

		// do transform
		auto transform = pimpl->rigidBody.getTransform().to(
				state.getTransform());
		state.transform(transform);

		// model
		pimpl->modelAsUpdateNode->update(state);

		state.popState();
	}
}

/**
 *
 * @return
 */
bool SgRigidBody::valid() const {
	return pimpl->valid;
}

/**
 *
 * @param vb
 */
OVERRIDE void SgRigidBody::visualize(render::ViewBuilder & vb) {
	if (pimpl->valid == false) {
		return;
	}

	// debug collisions
	if (vb.showCollisions()) {
		vb.pushState();
		auto & state = vb.getState();

		// do transform
		state.transform(pimpl->renderTransform);

		// debug
		std::vector<DebugGeometry> debug;
		pimpl->rigidBody.getCollision().draw(debug, state.getTransform());
		vb.addDebugGeometry(debug);

		vb.popState();
	}

	if (pimpl->modelAsVisualizeNode != nullptr) {
		vb.pushState();

		// do transform
		vb.getState().transform(pimpl->renderTransform);

		// model
		pimpl->modelAsVisualizeNode->visualize(vb);

		vb.popState();
	}
}

/**
 * get script object factory for SgRigidBody
 *
 * @return  SgRigidBody factory
 */
STATIC const ScriptObjectPtr & SgRigidBody::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
