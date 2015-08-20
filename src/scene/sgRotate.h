#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../core/quat.h"

#include "../scripting/scriptObject.h"

class SgRotate final: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 *
	 * @param rotation
	 */
	SgRotate(const Quat & rotation);

	/**
	 * default destructor
	 */
	inline virtual ~SgRotate() = default;

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
	Quat getRotation() const;

	/**
	 *
	 * @param rotation
	 */
	void setRotation(const Quat & rotation);

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
	 * get script object factory for SgRotate
	 *
	 * @return  SgRotate factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	Quat m_rotation;
};

