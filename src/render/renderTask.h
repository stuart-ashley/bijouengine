#pragma once

#include "../core/rect.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class Color;

namespace render {
	class Canvas;
	class Texture;

	class RenderTask {
	public:

		/**
		 * destructor
		 */
		virtual ~RenderTask();

		void addDependencies(const std::unordered_set<int> & textureIds);

		void addDependency(int textureId);

		const Color & getBGColor() const;

		const std::string & getName() const;

		int getLevel() const;

		bool getOpaque() const;

		const Rect & getRect() const;

		Rect getScissor() const;

		const std::unordered_set<int> & getTextureDependencies() const;

		Rect getViewport() const;

		const std::vector<std::shared_ptr<Texture>> & getTargetTextures() const;

		bool isCubeFace() const;

		void setBGColor(const Color & col);

		void setOpaque(bool opaque);

		void setRect(const Rect & rect);

		virtual bool hasExecuted() const = 0;

		virtual void execute(Canvas & canvas) = 0;

	protected:

		RenderTask(const std::string & name,
				const std::shared_ptr<Texture> & texture, const Rect & rect,
				int level);

		RenderTask(const std::string & name,
				const std::vector<std::shared_ptr<Texture>> & textures,
				const Rect & rect, int level);

		RenderTask(const std::string & name,
				const std::shared_ptr<Texture> & textures, const Rect & rect,
				const std::unordered_set<int> & dependencies, int level);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}
