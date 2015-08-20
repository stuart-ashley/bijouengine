#include "renderGraph.h"

#include "canvas.h"
#include "renderTask.h"
#include "texture.h"
#include "textureManager.h"

#include <algorithm>
#include <limits>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace render;

namespace {
	struct TaskNode {
		std::unordered_set<std::shared_ptr<TaskNode>> dependents;
		std::unordered_set<std::shared_ptr<TaskNode>> dependencies;
		std::shared_ptr<RenderTask> task;
		bool orphan;

		TaskNode(const std::shared_ptr<RenderTask> & task) :
				task(task), orphan(true) {
		}

		bool dependsUpon(const std::shared_ptr<TaskNode> & node) {
			for (const auto & depenent : dependents) {
				if (depenent == node || depenent->dependsUpon(node)) {
					return true;
				}
			}
			return false;
		}
	};
}

struct RenderGraph::impl {
	/** for each texture a list of tasks the texture depends upon */
	std::unordered_map<int, std::vector<std::shared_ptr<TaskNode>>>textureDependencies;
	/** for each texture a list of tasks that depend upon it */
	std::unordered_map<int, std::vector<std::shared_ptr<TaskNode>>> textureDependents;
	/** all tasks */
	std::vector<std::shared_ptr<TaskNode>> taskList;

	std::mutex m_lock;

	~impl() {
		// cyclic dependency stops things unravelling on there own, hence
		for (const auto & node : taskList) {
			for (auto & dependent : node->dependents) {
				dependent->dependencies.erase(node);
			}
		}
	}

	/** pop task from list for execution */
	std::shared_ptr<RenderTask> pop() {
		int lowest = std::numeric_limits<int>().max();
		std::shared_ptr<TaskNode> bestNode = nullptr;
		for (const auto & node : taskList) {
			if (node->orphan) {
				continue;
			}
			int level = node->task->getLevel();
			if (node->dependencies.empty() && lowest > level) {
				lowest = level;
				bestNode = node;
			}
		}
		if (bestNode != nullptr) {
			for (auto & dependent : bestNode->dependents) {
				dependent->dependencies.erase(bestNode);
			}
			taskList.erase(
					std::remove(taskList.begin(), taskList.end(), bestNode),
					taskList.end());
			return bestNode->task;
		}
		return nullptr;
	}
};

/**
 * construct empty render graph
 */
RenderGraph::RenderGraph() :
		pimpl(new impl()) {
}

/**
 * destructor
 */
RenderGraph::~RenderGraph() {
}

/**
 * Add task to render graph
 *
 * @return true if this is a new task, false if not
 */
void RenderGraph::addTask(const std::shared_ptr<RenderTask> & task) {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);

	// add new node to task list
	auto newNode = std::make_shared<TaskNode>(task);
	pimpl->taskList.emplace_back(newNode);

	// add dependents to new node
	for (const auto & tex : task->getTargetTextures()) {
		int texid = tex->getId();
		if (tex == TextureManager::getInstance().getScreen()) {
			newNode->orphan = false;
		}
		// add task to the dependencies of target texture
		auto & dependencies = pimpl->textureDependencies[texid];
		dependencies.emplace_back(newNode);
		// the tasks dependent on target texture are now dependent on task
		auto it = pimpl->textureDependents.find(texid);
		if (it != pimpl->textureDependents.end()) {
			newNode->orphan = false;
			for (auto & node : it->second) {
				// new task is needed by node
				newNode->dependents.emplace(node);
				node->dependencies.emplace(newNode);
			}
		}
	}

	// add dependencies to new node
	for (const auto & texid : task->getTextureDependencies()) {
		// task dependent on texture, add task to list of texture dependents
		auto & dependents = pimpl->textureDependents[texid];
		dependents.emplace_back(newNode);
		// task depends upon all texture dependencies
		auto it = pimpl->textureDependencies.find(texid);
		if (it != pimpl->textureDependencies.end()) {
			for (auto & node : it->second) {
				// check non cyclic
				if (node->dependsUpon(newNode) == false) {
					// node is needed by new task
					node->orphan = false;
					node->dependents.emplace(newNode);
					newNode->dependencies.emplace(node);
				}
			}
		}
	}
}

/**
 * Execute the render graph
 */
void RenderGraph::execute() {
	Canvas canvas;

	while (true) {
		auto task = pimpl->pop();
		if (task == nullptr) {
			break;
		}
		task->execute(canvas);
		TextureManager::getInstance().freshen(task->getTextureDependencies());
	}

	for (const auto & g : pimpl->taskList) {
		if (g->orphan == false) {
			std::cerr << "Uncompleted task: " << g->task->getName()
					<< std::endl;
		} else {
			//std::cerr << "Orphaned task: " << g->task->getName() << std::endl;
		}
	}
}
