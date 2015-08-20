#include "collisionEvent.h"

#include "../core/intersection.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <unordered_map>

namespace {
	/*
	 *
	 */
	struct GetBody0: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			stack.push(std::make_shared<String>(collisionEvent->getBody0()));
		}
	};

	/*
	 *
	 */
	struct GetBody1: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			stack.push(std::make_shared<String>(collisionEvent->getBody1()));
		}
	};

	/*
	 *
	 */
	struct GetDepth: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			stack.push(std::make_shared<Real>(collisionEvent->getDepth()));
		}
	};

	/*
	 *
	 */
	struct GetNormal: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			stack.push(std::make_shared<Normal>(collisionEvent->getNormal()));
		}
	};

	/*
	 *
	 */
	struct GetPoint: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			stack.push(std::make_shared<Vec3>(collisionEvent->getPoint()));
		}
	};

	/*
	 *
	 */
	struct GetWorldNormal0: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			auto normal = collisionEvent->getNormal();
			collisionEvent->getBody0Transform().rotate(normal);

			stack.push(std::make_shared<Normal>(normal));
		}
	};

	/*
	 *
	 */
	struct GetWorldNormal1: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			auto normal = -collisionEvent->getNormal();
			collisionEvent->getBody0Transform().rotate(normal);

			stack.push(std::make_shared<Normal>(normal));
		}
	};

	/*
	 *
	 */
	struct GetWorldPoint0: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			auto point = collisionEvent->getPoint();
			collisionEvent->getBody0Transform().transformPoint(point);

			stack.push(std::make_shared<Vec3>(point));
		}
	};

	/*
	 *
	 */
	struct GetWorldPoint1: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto collisionEvent = std::static_pointer_cast<CollisionEvent>(
					self);

			auto point = collisionEvent->getPoint();
			collisionEvent->getBody0Transform().transformPoint(point);
			point.scaleAdd(collisionEvent->getDepth(),
					collisionEvent->getNormal(), point);

			stack.push(std::make_shared<Vec3>(point));
		}
	};
}

/**
 * constructor
 *
 * @param bodyName0        name of first body
 * @param bodyName1        name of second body
 * @param body0Transform   transform from first body space to world
 * @param intersection     intersection in first body space
 */
CollisionEvent::CollisionEvent(const std::string & bodyName0,
		const std::string & bodyName1, const Transform & body0Transform,
		const Intersection & intersection) :
				body0(bodyName0),
				body1(bodyName1),
				body0Transform(body0Transform),
				normal(intersection.getNormal()),
				point(intersection.getPoint()),
				depth(intersection.getDepth()) {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr CollisionEvent::getMember(
		ScriptExecutionState & execState, const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {

	{ "getBody0", std::make_shared<GetBody0>() },

	{ "getBody1", std::make_shared<GetBody1>() },

	{ "getDepth", std::make_shared<GetDepth>() },

	{ "getNormal", std::make_shared<GetNormal>() },

	{ "getPoint", std::make_shared<GetPoint>() },

	{ "getWorldNormal0", std::make_shared<GetWorldNormal0>() },

	{ "getWorldNormal1", std::make_shared<GetWorldNormal1>() },

	{ "getWorldPoint0", std::make_shared<GetWorldPoint0>() },

	{ "getWorldPoint1", std::make_shared<GetWorldPoint1>() },

	};

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}
