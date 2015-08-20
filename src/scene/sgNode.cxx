#include "sgNode.h"

#include "builder.h"
#include "updateState.h"

#include "../core/boundingBox.h"
#include "../core/color.h"
#include "../core/debugGeometry.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

#include <algorithm>
#include <cassert>
#include <mutex>

using namespace render;

namespace {
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			std::shared_ptr<BoundingBox> bounds = nullptr;
			std::vector<std::shared_ptr<UpdateNode>> updateNodes;
			std::vector<std::shared_ptr<TaskInitNode>> taskInitNodes;
			std::vector<std::shared_ptr<VisualizeNode>> visualizeNodes;

			for (unsigned i = 0; i < nArgs; ++i) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(BoundingBox)) {
					bounds = std::static_pointer_cast<BoundingBox>(arg);
				} else {
					auto unode = std::dynamic_pointer_cast<UpdateNode>(arg);
					auto tnode = std::dynamic_pointer_cast<TaskInitNode>(arg);
					auto vnode = std::dynamic_pointer_cast<VisualizeNode>(arg);
					bool isNode = false;
					if (unode != nullptr) {
						updateNodes.emplace_back(unode);
						isNode = true;
					}
					if (tnode != nullptr) {
						taskInitNodes.emplace_back(tnode);
						isNode = true;
					}
					if (vnode != nullptr) {
						visualizeNodes.emplace_back(vnode);
						isNode = true;
					}
					scriptExecutionAssert(isNode,
							"Require scene node as argument "
									+ std::to_string(i + 1));
				}
			}
			stack.push(
					std::make_shared<SgNode>(bounds, updateNodes, taskInitNodes,
							visualizeNodes));
		}
	};

	struct Append: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto arg = stack.top();
			stack.pop();

			scriptExecutionAssert(
					std::static_pointer_cast<SgNode>(self)->append(arg),
					"Require scene node as argument");
		}
	};

	struct Contains: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto arg = stack.top();
			stack.pop();

			if (std::static_pointer_cast<SgNode>(self)->contains(arg)) {
				stack.push(Bool::True());
			} else {
				stack.push(Bool::False());
			}
		}
	};

	struct Disable: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> &) const override {
			checkNumArgs(nArgs, 0);

			std::static_pointer_cast<SgNode>(self)->setEnabled(false);
		}
	};

	struct Enable: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> &) const override {
			checkNumArgs(nArgs, 0);

			std::static_pointer_cast<SgNode>(self)->setEnabled(true);
		}
	};

	struct GetBounds: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto node = std::static_pointer_cast<SgNode>(self);

			assert(node->getBounds() != nullptr);
			stack.push(std::make_shared<BoundingBox>(*node->getBounds()));
		}
	};

	struct IsEnabled: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			if (std::static_pointer_cast<SgNode>(self)->isEnabled()) {
				stack.push(Bool::True());
			} else {
				stack.push(Bool::False());
			}
		}
	};

	struct Remove: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto arg = stack.top();
			stack.pop();

			scriptExecutionAssert(
					std::static_pointer_cast<SgNode>(self)->remove(arg),
					"Require existing element as argument");
		}
	};
}

struct SgNode::impl {
	std::mutex lock;
	int updateId;
	int taskInitId;
	std::shared_ptr<BoundingBox> bounds;
	std::vector<std::shared_ptr<UpdateNode>> updateNodes;
	std::vector<std::shared_ptr<TaskInitNode>> taskInitNodes;
	std::vector<std::shared_ptr<VisualizeNode>> visualizeNodes;
	std::vector<ScriptObjectPtr> addNodes;
	std::vector<ScriptObjectPtr> delNodes;
	bool enabled;

	impl(const std::shared_ptr<BoundingBox> & bounds,
			const std::vector<std::shared_ptr<UpdateNode>> & updateNodes,
			const std::vector<std::shared_ptr<TaskInitNode>> & taskInitNodes,
			const std::vector<std::shared_ptr<VisualizeNode>> & visualizeNodes) :
					updateId(0),
					taskInitId(0),
					bounds(bounds),
					updateNodes(updateNodes),
					taskInitNodes(taskInitNodes),
					visualizeNodes(visualizeNodes),
					enabled(true) {

	}

	void debugBounds(ViewBuilder & vb) {
		std::vector<DebugGeometry> debug;
		debug.emplace_back(vb.getState().getTransform(), Color::red(), *bounds);
		vb.addDebugGeometry(debug);
	}
};

/**
 * constructor
 *
 * @param bounds
 * @param updateNodes
 * @param taskInitNodes
 * @param visualizeNodes
 */
SgNode::SgNode(const std::shared_ptr<BoundingBox> & bounds,
		const std::vector<std::shared_ptr<UpdateNode>>& updateNodes,
		const std::vector<std::shared_ptr<TaskInitNode>> & taskInitNodes,
		const std::vector<std::shared_ptr<VisualizeNode>> & visualizeNodes) :
		pimpl(new impl(bounds, updateNodes, taskInitNodes, visualizeNodes)) {
}

/**
 * destructor
 */
SgNode::~SgNode() {
}

/**
 *
 * @param node
 */
bool SgNode::append(ScriptObjectPtr & node) {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	if (std::dynamic_pointer_cast<UpdateNode>(node) != nullptr
			|| std::dynamic_pointer_cast<TaskInitNode>(node) != nullptr
			|| std::dynamic_pointer_cast<VisualizeNode>(node) != nullptr) {
		pimpl->addNodes.emplace_back(node);
		return true;
	}
	return false;
}

/**
 *
 * @param node
 */
bool SgNode::contains(ScriptObjectPtr & node) {
	auto unode = std::dynamic_pointer_cast<UpdateNode>(node);
	auto tnode = std::dynamic_pointer_cast<TaskInitNode>(node);
	auto vnode = std::dynamic_pointer_cast<VisualizeNode>(node);

	if (std::find(pimpl->updateNodes.begin(), pimpl->updateNodes.end(), unode)
			!= pimpl->updateNodes.end()) {
		return true;
	}

	if (std::find(pimpl->taskInitNodes.begin(), pimpl->taskInitNodes.end(),
			tnode) != pimpl->taskInitNodes.end()) {
		return true;
	}

	if (std::find(pimpl->visualizeNodes.begin(), pimpl->visualizeNodes.end(),
			vnode) != pimpl->visualizeNodes.end()) {
		return true;
	}

	return false;
}

/**
 *
 * @return
 */
std::shared_ptr<BoundingBox> SgNode::getBounds() const {
	return pimpl->bounds;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr SgNode::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "append", std::make_shared<Append>() },
			{ "contains", std::make_shared<Contains>() },
			{ "disable", std::make_shared<Disable>() },
			{ "enable", std::make_shared<Enable>() },
			{ "getBounds", std::make_shared<GetBounds>() },
			{ "isEnabled", std::make_shared<IsEnabled>() },
			{ "remove", std::make_shared<Remove>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 * is node enabled
 *
 * @return  true if enabled, false otherwise
 */
bool SgNode::isEnabled() const {
	return pimpl->enabled;
}

/**
 *
 * @param node
 */
bool SgNode::remove(ScriptObjectPtr & node) {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	auto unode = std::dynamic_pointer_cast<UpdateNode>(node);
	auto tnode = std::dynamic_pointer_cast<TaskInitNode>(node);
	auto vnode = std::dynamic_pointer_cast<VisualizeNode>(node);

	if (std::find(pimpl->updateNodes.begin(), pimpl->updateNodes.end(), unode)
			!= pimpl->updateNodes.end()) {
		pimpl->delNodes.emplace_back(node);
		return true;
	}

	if (std::find(pimpl->taskInitNodes.begin(), pimpl->taskInitNodes.end(),
			tnode) != pimpl->taskInitNodes.end()) {
		pimpl->delNodes.emplace_back(node);
		return true;
	}

	if (std::find(pimpl->visualizeNodes.begin(), pimpl->visualizeNodes.end(),
			vnode) != pimpl->visualizeNodes.end()) {
		pimpl->delNodes.emplace_back(node);
		return true;
	}

	return false;
}

/**
 * enable / disable
 *
 * @param enabled  true to enable, false to disable
 */
void SgNode::setEnabled(bool enabled) const {
	pimpl->enabled = enabled;
}

/**
 *
 * @param builder
 */
OVERRIDE void SgNode::taskInit(Builder & builder) {
	if (pimpl->enabled == false) {
		return;
	}
	// only once per frame (groups can be instanced)
	if (pimpl->taskInitId == builder.getId()) {
		return;
	}
	pimpl->taskInitId = builder.getId();

	builder.pushState();
	for (const auto & node : pimpl->taskInitNodes) {
		node->taskInit(builder);
	}
	builder.popState();
}

/**
 *
 * @param state
 */
OVERRIDE void SgNode::update(UpdateState & state) {
	if (pimpl->enabled == false) {
		return;
	}
	// only update once per frame (groups can be instanced)
	if (pimpl->updateId == state.getUpdateId()) {
		return;
	}
	pimpl->updateId = state.getUpdateId();

	{
		std::lock_guard<std::mutex> locker(pimpl->lock);

		for (const auto & e : pimpl->addNodes) {
			auto unode = std::dynamic_pointer_cast<UpdateNode>(e);
			auto tnode = std::dynamic_pointer_cast<TaskInitNode>(e);
			auto vnode = std::dynamic_pointer_cast<VisualizeNode>(e);
			if (unode != nullptr) {
				pimpl->updateNodes.emplace_back(unode);
			}
			if (tnode != nullptr) {
				pimpl->taskInitNodes.emplace_back(tnode);
			}
			if (vnode != nullptr) {
				pimpl->visualizeNodes.emplace_back(vnode);
			}
		}
		pimpl->addNodes.clear();

		for (const auto & e : pimpl->delNodes) {
			auto unode = std::dynamic_pointer_cast<UpdateNode>(e);
			auto tnode = std::dynamic_pointer_cast<TaskInitNode>(e);
			auto vnode = std::dynamic_pointer_cast<VisualizeNode>(e);
			if (unode != nullptr) {
				pimpl->updateNodes.erase(
						std::remove(pimpl->updateNodes.begin(),
								pimpl->updateNodes.end(), unode),
						pimpl->updateNodes.end());
			}
			if (tnode != nullptr) {
				pimpl->taskInitNodes.erase(
						std::remove(pimpl->taskInitNodes.begin(),
								pimpl->taskInitNodes.end(), tnode),
						pimpl->taskInitNodes.end());
			}
			if (vnode != nullptr) {
				pimpl->visualizeNodes.erase(
						std::remove(pimpl->visualizeNodes.begin(),
								pimpl->visualizeNodes.end(), vnode),
						pimpl->visualizeNodes.end());
			}
		}
		pimpl->delNodes.clear();
	}

	state.pushState();
	for (const auto & node : pimpl->updateNodes) {
		node->update(state);
	}
	state.popState();
}

/**
 *
 * @param vb
 */
OVERRIDE void SgNode::visualize(render::ViewBuilder & vb) {
	if (pimpl->enabled == false) {
		return;
	}
	if (pimpl->bounds != nullptr && vb.isVisible(*pimpl->bounds) == false) {
		return;
	}

	vb.pushState();
	if (pimpl->bounds != nullptr) {
		vb.getState().clipLights(*pimpl->bounds);
	}
	for (const auto & node : pimpl->visualizeNodes) {
		node->visualize(vb);
	}
	vb.popState();
}

/**
 * get script object factory for SgNode
 *
 * @return  SgNode factory
 */
STATIC const ScriptObjectPtr & SgNode::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
