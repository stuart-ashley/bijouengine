#pragma once

#include "updateNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

class CollisionHierarchy;

class SgCollision: public ScriptObject, public UpdateNode, public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param name
	 * @param collision
	 */
	SgCollision(const std::string & name, const CollisionHierarchy & collision);

	/**
	 * destructor
	 */
	~SgCollision();

	/**
	 *
	 * @param state
	 */
	void update(UpdateState & state) override;

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgCollision
	 *
	 * @return  SgCollision factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

