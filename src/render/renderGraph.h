#pragma once

#include <memory>

namespace render {
	class RenderTask;

	class RenderGraph {
	public:

		/**
		 * construct empty render graph
		 */
		RenderGraph();

		/**
		 * destructor
		 */
		~RenderGraph();

		/**
		 * Add task to render graph
		 *
		 * @return true if this is a new task, false if not
		 */
		void addTask(const std::shared_ptr<RenderTask> & task);

		/**
		 * Execute the render graph
		 */
		void execute();

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

