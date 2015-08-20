#pragma once

#include "taskInitNode.h"

#include "../render/abstractProjectedTexture.h"

#include "../scripting/scriptObject.h"

class SgProjectedTexture final: public ScriptObject, public TaskInitNode {
public:

	/**
	 * constructor
	 *
	 * @param abstractProjectedTexture
	 */
	SgProjectedTexture(
			const render::AbstractProjectedTexture & abstractProjectedTexture);

	/**
	 * default destructor
	 */
	inline virtual ~SgProjectedTexture() = default;

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 * get script object factory for SgProjectedTexture
	 *
	 * @return  SgProjectedTexture factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	render::AbstractProjectedTexture abstractProjectedTexture;
};
