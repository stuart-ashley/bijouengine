#include "updateState.h"

#include "builder.h"
#include "physics.h"
#include "sceneProgram.h"
#include "system.h"
#include "taskInitNode.h"
#include "updateNode.h"
#include "visualizeNode.h"

#include "../core/config.h"
#include "../core/convexHull.h"
#include "../core/debugGeometry.h"
#include "../core/frameRate.h"
#include "../core/ray.h"
#include "../core/rigidBody.h"
#include "../core/timer.h"

#include "../render/renderGraph.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameters.h"
#include "../scripting/path.h"
#include "../scripting/scriptExecutionState.h"
#include "../scripting/string.h"

#include <cassert>
#include <stack>
#include <string>

namespace {
	int counter;

	class State {
	public:
		State() = default;

		State(const State &) = default;

		Transform getBone(const std::string & name) {
			auto it = m_bones.find(name);
			if (it != m_bones.end()) {
				return it->second;
			}
			return Transform();
		}

		Quat getRotation() const {
			return m_transform.getRotation();
		}

		Transform getTransform() const {
			return m_transform;
		}

		Vec3 getTranslation() const {
			return m_transform.getTranslation();
		}

		void rotate(Quat rotation) {
			m_transform.rotate(rotation);
		}

		void setBones(
				const std::unordered_map<std::string, Transform> & bones) {
			m_bones = bones;
		}

		void transform(Transform t) {
			m_transform.transform(t);
		}

		void translate(const Vec3 & translation) {
			m_transform.translate(translation);
		}
	private:
		Transform m_transform;
		std::unordered_map<std::string, Transform> m_bones;
	};

	class AddTask: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto arg = stack.top();
			stack.pop();

			std::static_pointer_cast<UpdateState>(self)->addTask(arg);
		}
	};

	class GetEvents: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto type = getArg<String>("string", stack, 1).getValue();

			auto updateState = std::static_pointer_cast<UpdateState>(self);

			std::vector<ScriptObjectPtr> list;
			for (const auto & event : updateState->getEvents(type)) {
				list.emplace_back(event);
			}
			stack.push(std::make_shared<List>(list));
		}
	};

	struct SortedIntersections: public Physics::RayIntersectionCallback {
		std::vector<double> distances;
		std::vector<ScriptObjectPtr> intersections;

		~SortedIntersections() {
		}

		void addIntersection(
				const std::shared_ptr<Physics::RayIntersection> & intersection)
						override {
			if (intersection->getDistance() < 0) {
				return;
			}
			for (size_t i = 0, n = distances.size(); i < n; ++i) {
				if (intersection->getDistance() < distances[i]) {
					intersections.insert(intersections.begin() + i,
							intersection);
					distances.insert(distances.begin() + i,
							intersection->getDistance());
					return;
				}
			}
			intersections.emplace_back(intersection);
			distances.emplace_back(intersection->getDistance());
		}
	};

	struct RayIntersection: public Executable {

		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);
			auto ray = getArg<Ray>("Ray", stack, 1);

			SortedIntersections callback;
			auto updateState = std::static_pointer_cast<UpdateState>(self);
			updateState->rayIntersection(ray, callback);

			stack.push(std::make_shared<List>(callback.intersections));
		}
	};
}

struct UpdateState::impl {
	int updateId;

	/** events added this update */
	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> newEvents;
	/** events added last update */
	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> events;
	std::stack<State> state;
	/** tasks */
	std::vector<std::shared_ptr<UpdateNode>> tasksRequiringUpdate;
	std::vector<std::shared_ptr<TaskInitNode>> tasksRequiringInit;

	float timeStep;
	Timer timer;
	FrameRate updateRate;
	float frameRate;
	float renderRate;
	size_t lastPolyCount;
	std::unique_ptr<Physics> oldPhysics;
	std::unique_ptr<Physics> physics;
	unsigned width;
	unsigned height;

	impl(int width, int height) :
					updateId(0),
					timeStep(.01f),
					frameRate(0),
					renderRate(0),
					lastPolyCount(0),
					oldPhysics(new Physics()),
					physics(new Physics()),
					width(width),
					height(height) {
	}

	/*
	 *
	 */
	void addEvent(const std::string & type, const ScriptObjectPtr & data) {
		auto & list = newEvents[type];
		list.emplace_back(data);
	}

	/*
	 *
	 */
	void setSystemVars(std::shared_ptr<System> & systemInstance) {
		auto updateFps = std::make_shared<Real>(updateRate.getRate());
		auto renderFps = std::make_shared<Real>(renderRate);
		systemInstance->setMember("fps",
				updateFps->getFloat() < renderFps->getFloat() ?
						updateFps : renderFps);
		systemInstance->setMember("updateRate", updateFps);
		systemInstance->setMember("renderRate", renderFps);
		systemInstance->setMember("width", std::make_shared<Real>(width));
		systemInstance->setMember("height", std::make_shared<Real>(height));
		bool debug = Config::getInstance().getBoolean("debug");
		systemInstance->setMember("debug",
				debug ? Bool::True() : Bool::False());
		// time property
		float time = static_cast<float>(timer.get());
		systemInstance->setMember("time", std::make_shared<Real>(time));
		systemInstance->setMember("step", std::make_shared<Real>(timeStep));
		// poly count
		systemInstance->setMember("polyCount",
				std::make_shared<Real>(static_cast<double>(lastPolyCount)));
		// home
		systemInstance->setMember("home",
				std::make_shared<Path>(Config::getInstance().getString("home")));
	}
};

/**
 *  constructor
 *
 * @param width   screen width
 * @param height  screen height
 */
UpdateState::UpdateState(int width, int height) :
		pimpl(new impl(width, height)) {
}

/**
 * destructor
 */
UpdateState::~UpdateState() {
}

/**
 * Add collision to state
 *
 * @param collision  collision to add
 */
void UpdateState::addCollision(const std::string & name,
		const Transform & previous, const Transform & current,
		const CollisionHierarchy & collision) {
	pimpl->physics->addCollision(name, previous, current, collision);
}

/**
 * Add constraint to state
 *
 * @param constraint  constraint to add
 */
void UpdateState::addConstraint(const Constraint & constraint) {
	pimpl->physics->addConstraint(constraint);
}

/**
 * Add end effector to state
 *
 * @param endEffector  end effector to add
 */
void UpdateState::addEndEffector(const EndEffector & endEffector) {
	pimpl->physics->addEndEffector(endEffector);
}

/**
 * Add list of events to state
 *
 * @param events  list of events to add
 */
void UpdateState::addEvents(
		const std::unordered_map<std::string, std::vector<ScriptObjectPtr>> & events) {
	for (const auto & entry : events) {
		auto & list = pimpl->newEvents[entry.first];
		list.insert(list.end(), entry.second.begin(), entry.second.end());
	}
}

/**
 * Add body to state
 *
 * @param rigidBody  body to add
 */
void UpdateState::addRigidBody(const RigidBody & rigidBody) {
	pimpl->physics->addRigidBody(rigidBody);
}

/**
 * add task to state, ( for scripting )
 *
 * @param node  task node to add
 */
void UpdateState::addTask(const ScriptObjectPtr & node) {
	scriptExecutionAssert(
			std::dynamic_pointer_cast<VisualizeNode>(node) == nullptr,
			"Require task as parameter");

	auto initNode = std::dynamic_pointer_cast<TaskInitNode>(node);
	scriptExecutionAssert(initNode != nullptr, "Require task as parameter");
	pimpl->tasksRequiringInit.emplace_back(initNode);

	auto updateNode = std::dynamic_pointer_cast<UpdateNode>(node);
	if (updateNode != nullptr) {
		pimpl->tasksRequiringUpdate.emplace_back(updateNode);
	}
}

/**
 * get named bone transform
 *
 * @param name  name of bone
 *
 * @return      transform for bone
 */
Transform UpdateState::getBoneTransform(const std::string & name) const {
	return pimpl->state.top().getBone(name);
}

/**
 * get events for named type
 *
 * @param name  type of events to get
 *
 * @return      events for named type
 */
const std::vector<ScriptObjectPtr> & UpdateState::getEvents(
		const std::string & name) const {
	return pimpl->events[name];
}

/**
 * get update id
 *
 * @return  update id
 */
int UpdateState::getUpdateId() const {
	return pimpl->updateId;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
std::shared_ptr<ScriptObject> UpdateState::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "addTask", std::make_shared<AddTask>() },
			{ "getEvents", std::make_shared<GetEvents>() },
			{ "rayIntersection", std::make_shared<RayIntersection>() } };
	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * get current rotation
 *
 * @return  current rotation
 */
Quat UpdateState::getRotation() const {
	return pimpl->state.top().getRotation();
}

/**
 * get current time step
 *
 * @return  current time step
 */
float UpdateState::getTimeStep() const {
	return pimpl->timeStep;
}

/**
 * get current transform
 *
 * @return  current transform
 */
Transform UpdateState::getTransform() const {
	return pimpl->state.top().getTransform();
}

/**
 * get current translation
 *
 * @return  current translation
 */
Vec3 UpdateState::getTranslation() const {
	return pimpl->state.top().getTranslation();
}

/**
 * push the current state
 */
void UpdateState::pushState() {
	pimpl->state.push(State(pimpl->state.top()));
}

/**
 * pop the current state
 */
void UpdateState::popState() {
	pimpl->state.pop();
}

/**
 *
 * @param ray
 * @param callback
 */
void UpdateState::rayIntersection(const Ray & ray,
		Physics::RayIntersectionCallback & callback) {
	pimpl->oldPhysics->rayIntersection(ray, callback);
}

/**
 * rotate current state
 *
 * @param rotation  rotation to apply
 */
void UpdateState::rotate(const Quat & rotation) {
	pimpl->state.top().rotate(rotation);
}

/**
 * Replace current bone transforms with new ones
 *
 * @param transforms
 *            bone transforms
 */
void UpdateState::setBoneTransforms(
		const std::unordered_map<std::string, Transform> & transforms) {
	pimpl->state.top().setBones(transforms);
}

/**
 * set time step for physics
 *
 * @param timeStep  new time step
 */
void UpdateState::setTimeStep(float timeStep) {
	pimpl->timeStep = timeStep;
}

/**
 * set render rate ( for informational purposes )
 *
 * @param renderRate  render rate
 */
void UpdateState::setRenderRate(float renderRate) {
	pimpl->renderRate = renderRate;
}

/**
 * transform current state
 *
 * @param t  transform to apply
 */
void UpdateState::transform(const Transform & t) {
	pimpl->state.top().transform(t);
}

/**
 * translate current state
 *
 * @param translation  translation to apply
 */
void UpdateState::translate(const Vec3 & translation) {
	pimpl->state.top().translate(translation);
}

/**
 * update scene graph generating render graph
 *
 * @param script  script to run
 *
 * @return        generated render graph
 */
std::shared_ptr<render::RenderGraph> UpdateState::update(
		const std::shared_ptr<SceneProgram> & script) {
	pimpl->updateId = ++counter;

	while (pimpl->state.empty() == false) {
		pimpl->state.pop();
	}
	pimpl->state.push(State());
	pimpl->events.swap(pimpl->newEvents);
	pimpl->newEvents.clear();

	// swap old and new physics
	pimpl->physics.swap(pimpl->oldPhysics);
	pimpl->physics = std::unique_ptr<Physics>(new Physics());

	pimpl->tasksRequiringUpdate.clear();
	pimpl->tasksRequiringInit.clear();

	std::vector<DebugGeometry> debugGeometry;

	if (script != nullptr && script->valid()) {
		// update System variables
		ScriptExecutionState execState;
		auto system = std::static_pointer_cast<System>(
				script->getMember(execState, "System"));
		pimpl->setSystemVars(system);

		// execute script
			script->execute(shared_from_this());

		for (auto & node : pimpl->tasksRequiringUpdate) {
			node->update(*this);
		}

		float speed = Config::getInstance().getFloat("simulationSpeed");
		addEvents(pimpl->physics->resolve(speed, pimpl->timeStep));
	}

	assert(pimpl->state.size() == 1);

	pimpl->updateRate.update();

	Builder builder(debugGeometry, pimpl->tasksRequiringInit);
	auto & rg = builder.execute();
	pimpl->lastPolyCount = builder.getPolyCount();

	return rg;
}
