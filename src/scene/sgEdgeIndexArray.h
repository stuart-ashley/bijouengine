#pragma once

#include "visualizeNode.h"

#include "../core/indexArray.h"

#include "../scripting/scriptObject.h"

#include <memory>

namespace render {
	class ViewBuilder;
}

class SgEdgeIndexArray final: public ScriptObject, public VisualizeNode {
public:

	/**
	 * Construct from index array
	 *
	 * @param array  array of edge indices
	 */
	SgEdgeIndexArray(const IndexArray & array);

	/**
	 * default destructor
	 */
	inline virtual ~SgEdgeIndexArray() = default;

	/**
	 * Add index array to render graph
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgEdgeIndexArray
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            SgEdgeIndexArray factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	IndexArray m_array;
};

