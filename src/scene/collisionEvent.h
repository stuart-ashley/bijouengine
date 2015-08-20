#pragma once

#include "../core/normal.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include "../scripting/scriptObject.h"

#include <string>

class Intersection;

class CollisionEvent final: public ScriptObject {
public:

	/**
	 * constructor
	 *
	 * @param bodyName0        name of first body
	 * @param bodyName1        name of second body
	 * @param body0Transform   transform from first body space to world
	 * @param intersection     intersection in first body space
	 */
	CollisionEvent(const std::string & bodyName0, const std::string & bodyName1,
			const Transform & body0Transform,
			const Intersection & intersection);

	inline virtual ~CollisionEvent() = default;

	inline const std::string & getBody0() const {
		return body0;
	}

	inline const Transform & getBody0Transform() const {
		return body0Transform;
	}

	inline const std::string & getBody1() const {
		return body1;
	}

	inline double getDepth() const {
		return depth;
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

	inline const Normal & getNormal() const {
		return normal;
	}

	inline const Vec3 & getPoint() const {
		return point;
	}

private:
	std::string body0;
	std::string body1;
	Transform body0Transform;
	Normal normal;
	Vec3 point;
	double depth;
};

