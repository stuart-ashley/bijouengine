#pragma once

#include "renderTask.h"

#include <memory>

namespace render {
	class Font;

	class Label: public RenderTask {
	public:

		Label(const std::string & name, const std::shared_ptr<Font> & font,
				const std::string & justify, const Rect & rect,
				const std::shared_ptr<Texture> & texture, const Color & color,
				bool opaque, const Color & bgColor, const std::string & text,
				int level);

		/**
		 * destructor
		 */
		virtual ~Label();

		void execute(Canvas & canvas);

		bool hasExecuted() const;

		void setText(const std::string & text);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

