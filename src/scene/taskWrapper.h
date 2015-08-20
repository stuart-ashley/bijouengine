#pragma once

#include "../render/renderTask.h"

class TaskWrapper {
public:
	TaskWrapper(const std::shared_ptr<render::RenderTask> & task) :
			task(task) {
	}

	inline virtual ~TaskWrapper() {
	}

	inline virtual void init() {
	}

	const std::shared_ptr<render::RenderTask> & getRenderTask() const {
		return task;
	}
private:
	std::shared_ptr<render::RenderTask> task;
};

