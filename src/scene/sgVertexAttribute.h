#pragma once

#include "visualizeNode.h"

#include "../render/vertexAttribute.h"

#include "../scripting/scriptObject.h"

namespace render {
	class ViewBuilder;
}

class SgVertexAttribute final: public ScriptObject, public VisualizeNode {
public:

	/**
	 *
	 * @param attribute
	 */
	SgVertexAttribute(const render::VertexAttribute & attribute);

	/**
	 * destructor
	 */
	inline virtual ~SgVertexAttribute() = default;

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgVertexAttribute
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            SgVertexAttribute factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	render::VertexAttribute m_attribute;
};

