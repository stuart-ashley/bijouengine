#pragma once

#include "taskInitNode.h"

#include "../core/abstractSunlight.h"

#include "../scripting/scriptObject.h"

class SgSunlight final: public ScriptObject, public TaskInitNode {
public:

	/**
	 *
	 * @param sunlight
	 */
	SgSunlight(const AbstractSunlight & sunlight);

	/**
	 * default destructor
	 */
	inline virtual ~SgSunlight() = default;

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 * get script object factory for SgSunlight
	 *
	 * @return  SgSunlight factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	AbstractSunlight light;
};

