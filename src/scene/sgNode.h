#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

#include <vector>

class BoundingBox;

namespace render {
	class ViewBuilder;
}

class SgNode: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param bounds
	 * @param updateNodes
	 * @param taskInitNodes
	 * @param visualizeNodes
	 */
	SgNode(const std::shared_ptr<BoundingBox> & bounds,
			const std::vector<std::shared_ptr<UpdateNode>>& updateNodes,
			const std::vector<std::shared_ptr<TaskInitNode>> & taskInitNodes,
			const std::vector<std::shared_ptr<VisualizeNode>> & visualizeNodes);

	/**
	 * destructor
	 */
	~SgNode();

	/**
	 *
	 * @param node
	 */
	bool append(ScriptObjectPtr & node);

	/**
	 *
	 * @param node
	 */
	bool contains(ScriptObjectPtr & node);

	/**
	 *
	 * @return
	 */
	std::shared_ptr<BoundingBox> getBounds() const;

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
	 * is node enabled
	 *
	 * @return  true if enabled, false otherwise
	 */
	bool isEnabled() const;

	/**
	 *
	 * @param node
	 */
	bool remove(ScriptObjectPtr & node);

	/**
	 * enable / disable
	 *
	 * @param enabled  true to enable, false to disable
	 */
	void setEnabled(bool enabled) const;

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
	 * get script object factory for SgNode
	 *
	 * @return  SgNode factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

