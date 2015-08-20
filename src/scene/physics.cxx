#include "physics.h"

#include "collisionEvent.h"

#include "../core/boundingBox.h"
#include "../core/bucket3d.h"
#include "../core/collisionHierarchy.h"
#include "../core/constraint.h"
#include "../core/endEffector.h"
#include "../core/intersection.h"
#include "../core/ray.h"
#include "../core/rigidBody.h"

#include "../scripting/real.h"
#include "../scripting/string.h"

#include <cassert>

namespace {
	struct CollisionObject {
		std::string name;
		Transform transform;
		CollisionHierarchy hierarchy;
		bool moved;

		CollisionObject(const std::string & name, const Transform & previous,
				const Transform & current, const CollisionHierarchy & hierarchy) :
						name(name),
						transform(current),
						hierarchy(hierarchy),
						moved(previous != current) {
		}
	};

	struct BodyCol {
		RigidBody body;
		std::vector<ScriptObjectPtr> & collisionEvents;

		BodyCol(const RigidBody & body,
				std::vector<ScriptObjectPtr> & collisionEvents) :
				body(body), collisionEvents(collisionEvents) {
		}

		void operator()(const CollisionObject & collision) {
			if (body.noCollide(collision.name)) {
				return;
			}
			Transform transform = body.getTransform();

			auto intersections = body.getCollision().collide(
					collision.hierarchy, collision.transform.to(transform));
			for (const auto & intersection : intersections) {
				collisionEvents.emplace_back(
						std::make_shared<CollisionEvent>(body.getName(),
								collision.name, transform, intersection));
			}
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
OVERRIDE ScriptObjectPtr Physics::RayIntersection::getMember(
		ScriptExecutionState & execState, const std::string & name2) const {
	if (name2 == "name") {
		return std::make_shared<String>(name);
	} else if (name2 == "point") {
		return std::make_shared<Vec3>(point);
	} else if (name2 == "distance") {
		return std::make_shared<Real>(distance);
	}
	return ScriptObject::getMember(execState, name2);
}

struct Physics::impl {

	std::unordered_map<std::string, RigidBody> bodies;
	std::vector<Constraint> constraints;
	std::unordered_map<std::string, std::unordered_set<std::string>> constraintMap;
	std::vector<CollisionObject> collisions;
	std::vector<EndEffector> endEffectors;

	/**
	 * Are named bodies locked in a constraint
	 *
	 * @param a  name of first body
	 * @param b  name of second body
	 * @return   true if bodies constrained, false otherwise
	 */
	bool constrained(const std::string & a, const std::string & b) {
		auto it = constraintMap.find(a);
		if (it == constraintMap.end()) {
			return false;
		}
		return it->second.find(b) != it->second.end();
	}

	/**
	 * Do named bodies collide with each other? not if they are both static, ie
	 * infinite mass, and not if they are members of a constraint
	 *
	 * @param a
	 *            first name
	 * @param b
	 *            second name
	 * @return true if collision allowed, false if not allowed
	 */
	bool collidable(const RigidBody & rba, const RigidBody & rbb) {
		if (rba.getInverseMass() == 0 && rbb.getInverseMass() == 0) {
			return false;
		}
		if (rba.noCollide(rbb.getName()) || rbb.noCollide(rba.getName())) {
			return false;
		}
		return !constrained(rba.getName(), rbb.getName());
	}

	/**
	 * Build map of constraints for fast checking
	 */
	void buildConstraintMap() {
		for (const auto & constraint : constraints) {
			constraintMap[constraint.getBodyName0()].emplace(
					constraint.getBodyName1());
			constraintMap[constraint.getBodyName1()].emplace(
					constraint.getBodyName0());
		}
	}

	// inverse kinematics using jacobian transpose
	void ik(const std::string & parent, const Vec3 & end, const Vec3 & goal,
			float s) {
		auto it = bodies.find(parent);
		if (it == bodies.end()) {
			return;
		}
		auto & body = it->second;

		Vec3 delta = (goal - end) * s;

		for (const auto & c : constraints) {
			if (c.getBodyName1() == parent) {
				body.setAngularVelocity(0.0, 0.0, 0.0);
				body.setLinearVelocity(0.0, 0.0, 0.0);
				// rotation
				Vec3 t = c.getPivot1Pos();
				Transform bodyTransform = body.getTransform();
				bodyTransform.transformPoint(t);
				Vec3 u = end - t;
				Vec3 v = goal - t;
				Vec3 w = u.cross(v);
				Normal axis = w.normalized();
				v.cross(axis, u);
				double d = v.dot(delta);
				d = std::max(-0.01, std::min(0.01, d));
				Quat r(axis, (float) d);
				r *= body.getRotation();
				body.setRotation(r);
				// next
				ik(c.getBodyName0(), end, goal, s);
			}
		}
	}

};

/**
 * constructor
 */
Physics::Physics() :
		pimpl(new impl()) {
}

/**
 * destructor
 */
Physics::~Physics() {
}

/**
 * Resolve collisions and constraints
 */
std::unordered_map<std::string, std::vector<ScriptObjectPtr>> Physics::resolve(
		float speed, float timeStep) {
	pimpl->buildConstraintMap();

	// wait for all bodies to be loaded
	for (auto & entry : pimpl->bodies) {
		auto & body = entry.second;
		if (body.validate() == false) {
			return std::unordered_map<std::string, std::vector<ScriptObjectPtr>>();
		}
	}

	BoundingBox bounds = BoundingBox::empty();
	double maxRadius = 0;

	// size up bucket
	for (const auto & entry : pimpl->bodies) {
		const auto & body = entry.second;
		bounds += body.getTranslation();
		maxRadius = std::max(maxRadius, body.getRadius());
	}
	for (const auto & collision : pimpl->collisions) {
		bounds += collision.transform.getTranslation();
		maxRadius = std::max(maxRadius,
				collision.hierarchy.getBounds().getRadius());
	}

	// nothing to do
	if (bounds.isEmpty()) {
		return std::unordered_map<std::string, std::vector<ScriptObjectPtr>>();
	}

	std::vector<ScriptObjectPtr> collisionEvents;

	for (int iter = 0, n = 10; iter < n; ++iter) {
		float ts = speed * timeStep / static_cast<float>(n);

		// inverse kinematics
		for (const auto & e : pimpl->endEffectors) {
			auto it = pimpl->bodies.find(e.getParent());
			if (it == pimpl->bodies.end()) {
				continue;
			}
			auto body = it->second;

			Transform pivotToWorld = body.getTransform();
			pivotToWorld.transform(e.getPivot());
			Vec3 end = pivotToWorld.getTranslation();

			// move toward
			Vec3 d = e.getGoalTranslation() - end;
			Vec3 t = body.getTranslation();
			t.scaleAdd(std::min(1.f, ts * e.getConvergence()), d, t);
			body.setTranslation(t);

			// rotate toward
			// pivot rotation
			Quat q = e.getPivot().getRotation();
			// body rotation
			Quat r = body.getRotation();
			// world space pivot rotation
			r *= q;
			// interpolate to goal position
			r.interpolate(e.getGoalRotation(),
					std::min(1.f, ts * e.getConvergence()));
			// undo pivot rotation
			r *= q.conjugate();
			// new body rotation
			body.setRotation(r);

			// joints
			pimpl->ik(e.getParent(), end, e.getGoalTranslation(),
					std::min(.0005f, ts * e.getConvergence()));
		}

		// create bodies bucket
		Bucket3d<RigidBody> bodies(bounds, maxRadius);

		for (auto & entry : pimpl->bodies) {
			auto & body = entry.second;
			// step
			body.step(ts);

			// insert into bucket
			bodies.insert(body, body.getTranslation(), body.getRadius());
		}

		// constraints
		for (const auto & constraint : pimpl->constraints) {
			auto it = pimpl->bodies.find(constraint.getBodyName0());
			if (it == pimpl->bodies.end()) {
				continue;
			}
			auto & a = it->second;

			it = pimpl->bodies.find(constraint.getBodyName1());
			if (it == pimpl->bodies.end()) {
				continue;
			}
			auto & b = it->second;

			if ((constraint.getLimitFlags() & 7) != 0) {
				a.fixConstraintTranslation(b, constraint);
				a.fixConstraintVelocity(b, constraint);
			}
			if ((constraint.getLimitFlags() & 56) != 0) {
				a.fixConstraintRotation(b, constraint);
			}
		}

		for (auto & objs : bodies.getBuckets()) {
			// collisions
			for (size_t i = 0, nBodies = objs.size(); i < nBodies; ++i) {
				auto & iBody = objs[i];
				for (size_t j = i + 1; j < nBodies; ++j) {
					auto & jBody = objs[j];
					assert (iBody != jBody);

					if (pimpl->collidable(iBody, jBody) == false) {
						// not collidable
						continue;
					}

					auto intersections = iBody.resolveCollision(jBody);
					if (intersections.size() == 0) {
						// no collision
						continue;
					}

					for (const auto & intersection : intersections) {
						collisionEvents.emplace_back(
								std::make_shared<CollisionEvent>(
										iBody.getName(), jBody.getName(),
										iBody.getTransform(), intersection));
					}
				}
			}
		}
	}

	// create collisions bucket
	Bucket3d<CollisionObject> collisions(bounds, maxRadius);

	for (const auto & collision : pimpl->collisions) {
		collisions.insert(collision, collision.transform.getTranslation(),
				collision.hierarchy.getBounds().getRadius());
	}

	// collisions
	for (const auto & entry : pimpl->bodies) {
		const auto & body = entry.second;
		if (body.getInverseMass() == 0) {
			continue;
		}
		BodyCol cb(body, collisionEvents);
		collisions.intersection(body.getTranslation(), body.getRadius(), cb);
	}

	for (auto & objs : collisions.getBuckets()) {
		for (size_t i = 0, n = objs.size(); i < n; ++i) {
			auto & a = objs[i];
			for (size_t j = i + 1; j < n; ++j) {
				auto & b = objs[j];
				if (a.moved == false && b.moved == false) {
					continue;
				}
				auto intersections = a.hierarchy.collide(b.hierarchy,
						b.transform.to(a.transform));
				for (const auto & intersection : intersections) {
					collisionEvents.emplace_back(
							std::make_shared<CollisionEvent>(a.name, b.name,
									a.transform, intersection));
				}
			}
		}
	}

	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> events;
	events["collision"] = collisionEvents;
	return events;
}

/**
 * Intersect ray against rigid bodies
 *
 * @param ray       ray to intersect
 * @param callback  callback for when an intersection occurs
 */
void Physics::rayIntersection(const Ray & ray,
		RayIntersectionCallback & callback) {
	for (const auto & entry : pimpl->bodies) {
		const auto & body = entry.second;
		Vec3 p;
		double d;
		if (body.rayIntersection(ray, p, d)) {
			callback.addIntersection(
					std::make_shared<RayIntersection>(body.getName(), p, d));
		}
	}

	for (const auto & collision : pimpl->collisions) {
		Ray rayInCollisionSpace = ray;
		rayInCollisionSpace.transform(collision.transform.inverse());
		Vec3 p;
		double d;
		if (collision.hierarchy.rayIntersection(rayInCollisionSpace, p, d)) {
			// transfrom point out of collision space
			collision.transform.transformPoint(p);
			callback.addIntersection(
					std::make_shared<RayIntersection>(collision.name, p, d));
		}
	}
}

/**
 * Add body to state
 *
 * @param rigidBody
 *            body to add
 */
void Physics::addRigidBody(const RigidBody & rigidBody) {
	pimpl->bodies.emplace(
			std::pair<std::string, RigidBody>(rigidBody.getName(), rigidBody));
}

/**
 * Add collision to state
 *
 * @param collision
 *            collision to add
 */
void Physics::addCollision(const std::string & name, const Transform & previous,
		const Transform & current, const CollisionHierarchy & collision) {
	pimpl->collisions.emplace_back(name, previous, current, collision);
}

/**
 * Add constraint to state
 *
 * @param constraint
 *            constraint to add
 */
void Physics::addConstraint(const Constraint & constraint) {
	pimpl->constraints.emplace_back(constraint);
}

/**
 * Add constraint to state
 *
 * @param constraint
 *            constraint to add
 */
void Physics::addEndEffector(const EndEffector & endEffector) {
	pimpl->endEffectors.emplace_back(endEffector);
}
