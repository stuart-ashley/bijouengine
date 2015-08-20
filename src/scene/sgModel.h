#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

#include <memory>

class SgNode;

class SgModel: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param defaultNode
	 * @param defaultLoresNode
	 * @param defaultWireNode
	 * @param defaultWireLoresNode
	 * @param shadowNode
	 * @param shadowLoresNode
	 * @param shadowWireNode
	 * @param shadowWireLoresNode
	 */
	SgModel(const std::shared_ptr<SgNode> & defaultNode,
			const std::shared_ptr<SgNode> & defaultLoresNode,
			const std::shared_ptr<SgNode> & defaultWireNode,
			const std::shared_ptr<SgNode> & defaultWireLoresNode,
			const std::shared_ptr<SgNode> & shadowNode,
			const std::shared_ptr<SgNode> & shadowLoresNode,
			const std::shared_ptr<SgNode> & shadowWireNode,
			const std::shared_ptr<SgNode> & shadowWireLoresNode);

	/**
	 * destructor
	 */
	~SgModel();

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
	 * get script object factory for SgModel
	 *
	 * @return  SgModel factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};
