#pragma once

#include "updateNode.h"

#include "../core/endEffector.h"

#include "../scripting/scriptObject.h"

class SgEndEffector final: public ScriptObject, public UpdateNode {
public:

	/**
	 * constructor
	 *
	 * @param endEffector
	 */
	inline SgEndEffector(const EndEffector & endEffector) :
			endEffector(endEffector) {
	}

	/**
	 * default destructor
	 */
	inline virtual ~SgEndEffector() = default;

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

	inline void setConvergence(float convergence) {
		endEffector.setConvergence(convergence);
	}

	inline void setGoalRotation(const Quat & rotation) {
		endEffector.setGoalRotation(rotation);
	}

	inline void setGoalTranslation(const Vec3 & translation) {
		endEffector.setGoalTranslation(translation);
	}

	/**
	 *
	 * @param state
	 */
	void update(UpdateState & state) override;

	/**
	 * get script object factory for SgEndEffector
	 *
	 * @return  SgEndEffector factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	EndEffector endEffector;
};

