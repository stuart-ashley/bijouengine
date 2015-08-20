#pragma once

#include "../core/vec3.h"

#include "../scripting/scriptObject.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class CollisionHierarchy;
class Constraint;
class DebugGeometry;
class EndEffector;
class Ray;
class RigidBody;
class Transform;

class Physics {
public:

	class RayIntersection: public ScriptObject {
	public:
		inline RayIntersection(const std::string & name, const Vec3 & point,
				double distance) :
				name(name), point(point), distance(distance) {
		}

		/**
		 * get distance along ray to intersection
		 *
		 * @return  distance to intersection
		 */
		inline double getDistance() const {
			return distance;
		}

		/**
		 * get named script object member
		 *
		 * @param execState  current script execution state
		 * @param name       name of member
		 *
		 * @return           script object represented by name
		 */
		ScriptObjectPtr getMember(ScriptExecutionState & execState,
				const std::string & name) const override;

	private:
		std::string name;
		Vec3 point;
		double distance;
	};

	class RayIntersectionCallback {
	public:
		inline virtual ~RayIntersectionCallback() {
		}

		virtual void addIntersection(
				const std::shared_ptr<RayIntersection> & intersection)=0;
	};

	/**
	 * constructor
	 */
	Physics();

	/**
	 * destructor
	 */
	~Physics();

	/**
	 * Add collision to state
	 *
	 * @param collision
	 *            collision to add
	 */
	void addCollision(const std::string & name, const Transform & previous,
			const Transform & current, const CollisionHierarchy & collision);

	/**
	 * Add constraint to state
	 *
	 * @param constraint
	 *            constraint to add
	 */
	void addConstraint(const Constraint & constraint);

	/**
	 * Add constraint to state
	 *
	 * @param constraint
	 *            constraint to add
	 */
	void addEndEffector(const EndEffector & endEffector);

	/**
	 * Add body to state
	 *
	 * @param rigidBody
	 *            body to add
	 */
	void addRigidBody(const RigidBody & rigidBody);

	/**
	 * Intersect ray against rigid bodies
	 *
	 * @param ray       ray to intersect
	 * @param callback  callback for when an intersection occurs
	 */
	void rayIntersection(const Ray & ray, RayIntersectionCallback & callback);

	/**
	 * Resolve collisions and constraints
	 */
	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> resolve(
			float speed, float timeStep);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

