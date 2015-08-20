#pragma once

#include "updateNode.h"

#include "../core/constraint.h"

#include "../scripting/scriptObject.h"

class SgConstraint final: public ScriptObject, public UpdateNode {
public:

	/**
	 * Construct from constraint
	 *
	 * @param constraint
	 */
	SgConstraint(const Constraint & constraint);

	/**
	 * default destructor
	 */
	inline virtual ~SgConstraint() = default;

	inline Constraint & getConstraint() {
		return constraint;
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

	/**
	 * On update add constraint to state
	 */
	void update(UpdateState & state) override;

	/**
	 * get script object factory for SgConstraint
	 *
	 * @return  SgConstraint factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	Constraint constraint;
};

