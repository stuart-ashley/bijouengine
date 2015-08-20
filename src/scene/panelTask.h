#pragma once

#include "updateNode.h"
#include "taskInitNode.h"

#include "../render/blendFlag.h"

#include "../scripting/scriptObject.h"

#include <string>
#include <vector>

namespace render {
	class ShaderTag;
	class Texture;
	class UniformArray;
}

class Animation;
class Color;
class Rect;

class PanelTask final: public ScriptObject, public UpdateNode, public TaskInitNode {
public:

	PanelTask(const std::string & name, const Rect & sRect, const Rect & dRect,
			const render::ShaderTag & shader,
			const std::shared_ptr<render::Texture> & texture,
			const std::shared_ptr<render::Texture> & source,
			render::BlendFlag blend, int level, const Color & color, float a,
			const render::UniformArray & uniforms,
			const std::vector<Animation> & animatedUniforms);

	/**
	 * destructor
	 */
	virtual ~PanelTask();

	const Rect & getDstRect() const;

	/**
	 * get named script object member
	 *
	 * @param execState  current script execution state
	 * @param name       name of member
	 *
	 * @return           script object represented by name
	 */
	ScriptObjectPtr getMember(ScriptExecutionState & execState,
			const std::string & name) const;

	void setAlpha(float alpha);

	void setBGColor(const Color & bgColor);

	void setDstRect(const Rect & dstRect);

	void setOpaque(bool opaque);

	void setSrcRect(const Rect & srcRect);

	/**
	 *
	 * @param builder
	 */
	void taskInit(Builder & builder) override;

	/**
	 *
	 */
	void update(UpdateState & state) override;

	/**
	 * get script object factory for PanelTask
	 *
	 * @return            PanelTask factory
	 */
	static ScriptObjectPtr getFactory();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

