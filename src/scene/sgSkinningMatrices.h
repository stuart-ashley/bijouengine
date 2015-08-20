#pragma once

#include "updateNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

#include <vector>

class SkinningMatrix;

class SgSkinningMatrices: public ScriptObject,
		public UpdateNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 */
	SgSkinningMatrices(const std::vector<SkinningMatrix> & matrices);

	/**
	 * destructor
	 */
	~SgSkinningMatrices();

	/**
	 *
	 */
	void update(UpdateState & state) override;

	/**
	 *
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgSkinningMatrices
	 *
	 * @return  SgSkinningMatrices factory
	 */
	static ScriptObjectPtr getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

