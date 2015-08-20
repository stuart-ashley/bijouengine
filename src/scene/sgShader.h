#pragma once

#include "visualizeNode.h"

#include "../render/shaderTag.h"

#include "../scripting/scriptObject.h"

namespace render {
	class ViewBuilder;
}

class SgShader final: public ScriptObject, public VisualizeNode {
public:

	SgShader(const render::ShaderTag & tag);

	/**
	 * default destructor
	 */
	inline virtual ~SgShader() = default;

	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgShader
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            SgShader factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	/**
	 * Process lights
	 *
	 * @param builder
	 */
	void processLights(const render::ViewBuilder & vb);

	render::ShaderTag shaderTag;
};

