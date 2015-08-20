#pragma once

#include "taskInitNode.h"

#include "../render/abstractProjectedKaleidoscope.h"

#include "../scripting/scriptObject.h"

class SgProjectedKaleidoscope final: public ScriptObject, public TaskInitNode {
public:

	/**
	 * constructor
	 *
	 * @param abstractProjectedKaleidoscope
	 */
	SgProjectedKaleidoscope(
			const render::AbstractProjectedKaleidoscope & abstractProjectedKaleidoscope);

	/**
	 * default destructor
	 */
	inline virtual ~SgProjectedKaleidoscope() = default;

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 * get script object factory for SgProjectedKaleidoscope
	 *
	 * @return  SgProjectedKaleidoscope factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	render::AbstractProjectedKaleidoscope abstractProjectedKaleidoscope;
};

