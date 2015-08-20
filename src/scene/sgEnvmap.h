#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

#include <vector>

namespace render {
	class Texture;
}

class SgEnvmap final: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param cubemap
	 * @param updateNodes
	 * @param taskInitNodes
	 * @param visualizeNodes
	 */
	SgEnvmap(std::shared_ptr<render::Texture> & cubemap,
			const std::vector<std::shared_ptr<UpdateNode>>& updateNodes,
			const std::vector<std::shared_ptr<TaskInitNode>> & taskInitNodes,
			const std::vector<std::shared_ptr<VisualizeNode>> & visualizeNodes);

	/*
	 * destructor
	 */
	virtual ~SgEnvmap();

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

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
	 * get script object factory for SgEnvmap
	 *
	 * @return  SgEnvmap factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

