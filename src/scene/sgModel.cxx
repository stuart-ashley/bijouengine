#include "sgModel.h"

#include "sgNode.h"

#include "../render/view.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/none.h"
#include "../scripting/parameters.h"

using namespace render;

namespace {

	std::vector<BaseParameter> params = {
			Parameter<SgNode>("default", None::none()),
			Parameter<SgNode>("defaultLores", None::none()),
			Parameter<SgNode>("defaultWire", None::none()),
			Parameter<SgNode>("defaultWireLores", None::none()),
			Parameter<SgNode>("shadow", None::none()),
			Parameter<SgNode>("shadowLores", None::none()),
			Parameter<SgNode>("shadowWire", None::none()),
			Parameter<SgNode>("shadowWireLores", None::none()) };

	/*
	 *
	 */
	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			std::shared_ptr<SgNode> defaultNode = nullptr;
			if (args["default"] != None::none()) {
				defaultNode = std::static_pointer_cast<SgNode>(args["default"]);
			}
			std::shared_ptr<SgNode> defaultLoresNode = nullptr;
			if (args["defaultLores"] != None::none()) {
				defaultLoresNode = std::static_pointer_cast<SgNode>(
						args["defaultLores"]);
			}
			std::shared_ptr<SgNode> defaultWireNode = nullptr;
			if (args["defaultWire"] != None::none()) {
				defaultWireNode = std::static_pointer_cast<SgNode>(
						args["defaultWire"]);
			}
			std::shared_ptr<SgNode> defaultWireLoresNode = nullptr;
			if (args["defaultWireLores"] != None::none()) {
				defaultWireLoresNode = std::static_pointer_cast<SgNode>(
						args["defaultWireLores"]);
			}
			std::shared_ptr<SgNode> shadowNode = nullptr;
			if (args["shadow"] != None::none()) {
				shadowNode = std::static_pointer_cast<SgNode>(args["shadow"]);
			}
			std::shared_ptr<SgNode> shadowLoresNode = nullptr;
			if (args["shadowLores"] != None::none()) {
				shadowLoresNode = std::static_pointer_cast<SgNode>(
						args["shadowLores"]);
			}
			std::shared_ptr<SgNode> shadowWireNode = nullptr;
			if (args["shadowWire"] != None::none()) {
				shadowWireNode = std::static_pointer_cast<SgNode>(
						args["shadowWire"]);
			}
			std::shared_ptr<SgNode> shadowWireLoresNode = nullptr;
			if (args["shadowWireLores"] != None::none()) {
				shadowWireLoresNode = std::static_pointer_cast<SgNode>(
						args["shadowWireLores"]);
			}

			stack.push(
					std::make_shared<SgModel>(defaultNode, defaultLoresNode,
							defaultWireNode, defaultWireLoresNode, shadowNode,
							shadowLoresNode, shadowWireNode,
							shadowWireLoresNode));

		}
	};
}

struct SgModel::impl {
	std::shared_ptr<SgNode> defaultNode;
	std::shared_ptr<SgNode> defaultLoresNode;
	std::shared_ptr<SgNode> defaultWireNode;
	std::shared_ptr<SgNode> defaultWireLoresNode;
	std::shared_ptr<SgNode> shadowNode;
	std::shared_ptr<SgNode> shadowLoresNode;
	std::shared_ptr<SgNode> shadowWireNode;
	std::shared_ptr<SgNode> shadowWireLoresNode;

	impl(const std::shared_ptr<SgNode> & defaultNode,
			const std::shared_ptr<SgNode> & defaultLoresNode,
			const std::shared_ptr<SgNode> & defaultWireNode,
			const std::shared_ptr<SgNode> & defaultWireLoresNode,
			const std::shared_ptr<SgNode> & shadowNode,
			const std::shared_ptr<SgNode> & shadowLoresNode,
			const std::shared_ptr<SgNode> & shadowWireNode,
			const std::shared_ptr<SgNode> & shadowWireLoresNode) :
					defaultNode(defaultNode),
					defaultLoresNode(defaultLoresNode),
					defaultWireNode(defaultWireNode),
					defaultWireLoresNode(defaultWireLoresNode),
					shadowNode(shadowNode),
					shadowLoresNode(shadowLoresNode),
					shadowWireNode(shadowWireNode),
					shadowWireLoresNode(shadowWireLoresNode) {
	}
};

/**
 * constructor
 *
 * @param defaultNode
 * @param defaultLoresNode
 * @param defaultWireNode
 * @param defaultWireLoresNode
 * @param shadowNode
 * @param shadowLoresNode
 * @param shadowWireNode
 * @param shadowWireLoresNode
 */
SgModel::SgModel(const std::shared_ptr<SgNode> & defaultNode,
		const std::shared_ptr<SgNode> & defaultLoresNode,
		const std::shared_ptr<SgNode> & defaultWireNode,
		const std::shared_ptr<SgNode> & defaultWireLoresNode,
		const std::shared_ptr<SgNode> & shadowNode,
		const std::shared_ptr<SgNode> & shadowLoresNode,
		const std::shared_ptr<SgNode> & shadowWireNode,
		const std::shared_ptr<SgNode> & shadowWireLoresNode) :
				pimpl(
						new impl(defaultNode, defaultLoresNode, defaultWireNode,
								defaultWireLoresNode, shadowNode,
								shadowLoresNode, shadowWireNode,
								shadowWireLoresNode)) {
}

/**
 * destructor
 */
SgModel::~SgModel(){
}

/**
 *
 * @param builder
 */
OVERRIDE void SgModel::taskInit(Builder & builder) {
	if (pimpl->defaultNode != nullptr) {
		pimpl->defaultNode->taskInit(builder);
	}
	if (pimpl->defaultLoresNode != nullptr) {
		pimpl->defaultLoresNode->taskInit(builder);
	}
	if (pimpl->defaultWireNode != nullptr) {
		pimpl->defaultWireNode->taskInit(builder);
	}
	if (pimpl->defaultWireLoresNode != nullptr) {
		pimpl->defaultWireLoresNode->taskInit(builder);
	}
	if (pimpl->shadowNode != nullptr) {
		pimpl->shadowNode->taskInit(builder);
	}
	if (pimpl->shadowLoresNode != nullptr) {
		pimpl->shadowLoresNode->taskInit(builder);
	}
	if (pimpl->shadowWireNode != nullptr) {
		pimpl->shadowWireNode->taskInit(builder);
	}
	if (pimpl->shadowWireLoresNode != nullptr) {
		pimpl->shadowWireLoresNode->taskInit(builder);
	}
}

/**
 *
 * @param state
 */
OVERRIDE void SgModel::update(UpdateState & state) {
	if (pimpl->defaultNode != nullptr) {
		pimpl->defaultNode->update(state);
	}
	if (pimpl->defaultLoresNode != nullptr) {
		pimpl->defaultLoresNode->update(state);
	}
	if (pimpl->defaultWireNode != nullptr) {
		pimpl->defaultWireNode->update(state);
	}
	if (pimpl->defaultWireLoresNode != nullptr) {
		pimpl->defaultWireLoresNode->update(state);
	}
	if (pimpl->shadowNode != nullptr) {
		pimpl->shadowNode->update(state);
	}
	if (pimpl->shadowLoresNode != nullptr) {
		pimpl->shadowLoresNode->update(state);
	}
	if (pimpl->shadowWireNode != nullptr) {
		pimpl->shadowWireNode->update(state);
	}
	if (pimpl->shadowWireLoresNode != nullptr) {
		pimpl->shadowWireLoresNode->update(state);
	}
}

/**
 *
 * @param vb
 */
OVERRIDE void SgModel::visualize(render::ViewBuilder & vb) {
	const auto & modifiers = vb.getView()->getModifiers();
	if (vb.getView()->isType(View::Type::SHADOW)) {
		if (modifiers[View::Modifier::WIREFRAME]
				&& modifiers[View::Modifier::LORES]
				&& pimpl->shadowWireLoresNode != nullptr) {
			pimpl->shadowWireLoresNode->visualize(vb);
			return;
		}
		if (modifiers[View::Modifier::WIREFRAME]
				&& pimpl->shadowWireNode != nullptr) {
			pimpl->shadowWireNode->visualize(vb);
			return;
		}
		if (modifiers[View::Modifier::LORES]
				&& pimpl->shadowLoresNode != nullptr) {
			pimpl->shadowLoresNode->visualize(vb);
			return;
		}
		if (pimpl->shadowNode != nullptr) {
			pimpl->shadowNode->visualize(vb);
		}
	} else {
		if (modifiers[View::Modifier::WIREFRAME]
				&& modifiers[View::Modifier::LORES]
				&& pimpl->defaultWireLoresNode != nullptr) {
			pimpl->defaultWireLoresNode->visualize(vb);
			return;
		}
		if (modifiers[View::Modifier::WIREFRAME]
				&& pimpl->defaultWireNode != nullptr) {
			pimpl->defaultWireNode->visualize(vb);
			return;
		}
		if (modifiers[View::Modifier::LORES]
				&& pimpl->defaultLoresNode != nullptr) {
			pimpl->defaultLoresNode->visualize(vb);
			return;
		}
		if (pimpl->defaultNode != nullptr) {
			pimpl->defaultNode->visualize(vb);
		}
	}
}

/**
 * get script object factory for SgModel
 *
 * @return  SgModel factory
 */
STATIC const ScriptObjectPtr & SgModel::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
