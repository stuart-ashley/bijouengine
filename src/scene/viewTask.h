#pragma once

#include "updateNode.h"
#include "taskInitNode.h"

#include "../core/ray.h"

#include "../render/view.h"

#include "../scripting/scriptObject.h"

class Color;
class SgCamera;
class SgNode;
class Vec2;

namespace render {
	class ShaderTag;
	class Texture;
	class UniformArray;
}

class ViewTask: public ScriptObject, public UpdateNode, public TaskInitNode {
public:

	/**
	 * constructor
	 *
	 * @param name
	 * @param camera
	 * @param textures
	 * @param rect
	 * @param uniforms
	 * @param shader
	 * @param cull
	 * @param depthCompare
	 * @param type
	 * @param modifiers
	 * @param level
	 * @param bgColor
	 * @param root
	 */
	ViewTask(const std::string & name, const std::shared_ptr<SgCamera> & camera,
			const std::vector<std::shared_ptr<render::Texture>> & textures,
			const Rect & rect, const render::UniformArray & uniforms,
			const render::ShaderTag & shader, render::View::Cull cull,
			render::View::DepthCompare depthCompare, render::View::Type type,
			const render::View::ModifierSet & modifiers, int level,
			const Color & bgColor, const std::shared_ptr<SgNode> & root);

	/**
	 * destructor
	 */
	~ViewTask();

	/**
	 *
	 * @param p
	 * @return
	 */
	Ray calculateRay(const Vec2 & p) const;

	/**
	 *
	 * @return
	 */
	const std::shared_ptr<SgCamera> & getCamera() const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const override;

	/**
	 *
	 * @return
	 */
	const Rect & getRect() const;

	/**
	 *
	 * @param camera
	 */
	void setCamera(const std::shared_ptr<SgCamera> & camera) const;

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
	 * get script object factory for ViewTask
	 *
	 * @return  ViewTask factory
	 */
	static const ScriptObjectPtr & getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

