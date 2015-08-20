#pragma once

#include "visualizeNode.h"

#include "../render/indexedTriangles.h"

#include "../scripting/scriptObject.h"

namespace render {
	class ViewBuilder;
}

class SgIndexedTriangles final: public ScriptObject, public VisualizeNode {
public:

	/**
	 * Construct from index array
	 */
	SgIndexedTriangles(const render::IndexedTriangles & indices);

	/**
	 * default destructor
	 */
	inline virtual ~SgIndexedTriangles() = default;

	/**
	 * Add index array to render graph
	 */
	void visualize(render::ViewBuilder & vb);

	/**
	 * get script object factory for SgIndexedTriangles
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            SgIndexedTriangles factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	render::IndexedTriangles indices;
	bool valid;
};

