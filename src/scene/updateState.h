#pragma once

#include "physics.h"

#include "../core/quat.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include "../scripting/scriptObject.h"

#include <memory>
#include <unordered_map>
#include <vector>

class CollisionHierarchy;
class Constraint;
class ConvexHull;
class EndEffector;
class Ray;
class RigidBody;
class SceneProgram;

namespace render {
	class RenderGraph;
}

class UpdateState: public std::enable_shared_from_this<UpdateState>,
		public ScriptObject {
public:

	/**
	 *  constructor
	 *
	 * @param width   screen width
	 * @param height  screen height
	 */
	UpdateState(int width, int height);

	/**
	 * destructor
	 */
	~UpdateState();

	/**
	 * Add collision to state
	 *
	 * @param collision  collision to add
	 */
	void addCollision(const std::string & name, const Transform & previous,
			const Transform & current, const CollisionHierarchy & collision);

	/**
	 * Add constraint to state
	 *
	 * @param constraint  constraint to add
	 */
	void addConstraint(const Constraint & constraint);

	/**
	 * Add end effector to state
	 *
	 * @param endEffector  end effector to add
	 */
	void addEndEffector(const EndEffector & endEffector);

	/**
	 * Add list of events to state
	 *
	 * @param events  list of events to add
	 */
	void addEvents(
			const std::unordered_map<std::string, std::vector<ScriptObjectPtr>> & events);

	/**
	 * Add body to state
	 *
	 * @param rigidBody  body to add
	 */
	void addRigidBody(const RigidBody & rigidBody);

	/**
	 * add task to state, ( for scripting )
	 *
	 * @param node  task node to add
	 */
	void addTask(const ScriptObjectPtr & node);

	/**
	 * get named bone transform
	 *
	 * @param name  name of bone
	 *
	 * @return      transform for bone
	 */
	Transform getBoneTransform(const std::string & name) const;

	/**
	 * get events for named type
	 *
	 * @param name  type of events to get
	 *
	 * @return      events for named type
	 */
	const std::vector<ScriptObjectPtr> & getEvents(
			const std::string & name) const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	std::shared_ptr<ScriptObject> getMember(ScriptExecutionState & execState,
			const std::string & name) const;

	/**
	 * get current rotation
	 *
	 * @return  current rotation
	 */
	Quat getRotation() const;

	/**
	 * get current time step
	 *
	 * @return  current time step
	 */
	float getTimeStep() const;

	/**
	 * get current transform
	 *
	 * @return  current transform
	 */
	Transform getTransform() const;

	/**
	 * get current translation
	 *
	 * @return  current translation
	 */
	Vec3 getTranslation() const;

	/**
	 * get update id
	 *
	 * @return  update id
	 */
	int getUpdateId() const;

	/**
	 * pop the current state
	 */
	void popState();

	/**
	 * push the current state
	 */
	void pushState();

	/**
	 *
	 * @param ray
	 * @param callback
	 */
	void rayIntersection(const Ray & ray,
			Physics::RayIntersectionCallback & callback);

	/**
	 * rotate current state
	 *
	 * @param rotation  rotation to apply
	 */
	void rotate(const Quat & rotation);

	/**
	 * Replace current bone transforms with new ones
	 *
	 * @param transforms
	 *            bone transforms
	 */
	void setBoneTransforms(
			const std::unordered_map<std::string, Transform> & transforms);

	/**
	 * set render rate ( for informational purposes )
	 *
	 * @param renderRate  render rate
	 */
	void setRenderRate(float renderRate);

	/**
	 * set time step for physics
	 *
	 * @param timeStep  new time step
	 */
	void setTimeStep(float timeStep);

	/**
	 * transform current state
	 *
	 * @param t  transform to apply
	 */
	void transform(const Transform & t);

	/**
	 * translate current state
	 *
	 * @param translation  translation to apply
	 */
	void translate(const Vec3 & translation);

	/**
	 * update scene graph generating render graph
	 *
	 * @param script  script to run
	 *
	 * @return        generated render graph
	 */
	std::shared_ptr<render::RenderGraph> update(
			const std::shared_ptr<SceneProgram> & script);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

