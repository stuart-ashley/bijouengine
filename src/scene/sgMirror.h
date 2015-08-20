#pragma once

#include "updateNode.h"
#include "taskInitNode.h"
#include "visualizeNode.h"

#include "../scripting/scriptObject.h"

namespace render {
	class ViewBuilder;
}

class Plane;

class SgMirror: public ScriptObject,
		public UpdateNode,
		public TaskInitNode,
		public VisualizeNode {
public:

	/**
	 * constructor
	 *
	 * @param name
	 * @param reflect
	 * @param refract
	 * @param plane
	 * @param recursion
	 * @param mesh
	 */
	SgMirror(const std::string & name, float reflect, float refract,
			const Plane & plane, size_t recursion,
			const ScriptObjectPtr & mesh);

	/**
	 * destructor
	 */
	~SgMirror();

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 *
	 * @param state
	 */
	void update(UpdateState & state) override;

	/**
	 *
	 * @param vb
	 */
	void visualize(render::ViewBuilder & vb) override;

	/**
	 * get script object factory for SgMirror
	 *
	 * @return  SgMirror factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

