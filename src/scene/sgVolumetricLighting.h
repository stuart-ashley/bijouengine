#pragma once

#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

class SgVolumetricLighting: public ScriptObject, public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param near
	 * @param far
	 * @param slices
	 * @param density
	 */
	SgVolumetricLighting(float near, float far, int slices, float density);

	/**
	 * destructor
	 */
	~SgVolumetricLighting();

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgVolumetricLighting
	 *
	 * @return  SgVolumetricLighting factory
	 */
	static ScriptObjectPtr getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

