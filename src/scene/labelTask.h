#pragma once

#include "taskInitNode.h"

#include "../scripting/scriptObject.h"

#include <memory>
#include <string>

class Builder;
class Color;
class Rect;

namespace render {
	class Font;
	class Texture;
}

class LabelTask final: public ScriptObject, public TaskInitNode {
public:

	LabelTask(const std::string & name,
			const std::shared_ptr<render::Font> & font, const Rect & rect,
			const std::shared_ptr<render::Texture> & texture, bool opaque,
			const Color & color, const Color & bgColor,
			const std::string & justify, const std::string & text, int level);

	/**
	 * destructor
	 */
	virtual ~LabelTask();

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

	const Rect & getRect() const;

	void setBgColor(const Color & bgColor);

	void setColor(const Color & color);

	void setOpaque(bool opaque);

	void setRect(const Rect & rect);

	void setText(const std::string & text);

	void taskInit(Builder & builder) override;

	/**
	 * get script object factory for LabelTask
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            LabelTask factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

