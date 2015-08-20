#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

class Quat;
class RigidBody;
class Vec3;

class SgRigidBody: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param rb
	 * @param model
	 */
	SgRigidBody(const RigidBody & rb, const ScriptObjectPtr & model);

	/**
	 * destructor
	 */
	~SgRigidBody();

	/**
	 *
	 */
	void freeze();

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

	/**
	 *
	 * @return
	 */
	const ScriptObjectPtr & getModel() const;

	/**
	 *
	 * @return
	 */
	RigidBody & getRigidBody() const;

	/**
	 *
	 * @param model
	 */
	void setModel(const ScriptObjectPtr & model) const;

	/**
	 *
	 * @param rotation
	 */
	void setRotation(const Quat & rotation) const;

	/**
	 *
	 * @param translation
	 */
	void setTranslation(const Vec3 & translation) const;

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 *
	 */
	void unfreeze();

	/**
	 *
	 * @param state
	 */
	void update(UpdateState & state) override;

	/**
	 *
	 * @return
	 */
	bool valid() const;

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgRigidBody
	 *
	 * @return  SgRigidBody factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

