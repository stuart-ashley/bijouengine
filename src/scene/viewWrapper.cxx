#include "viewWrapper.h"

#include "lights.h"
#include "sgNode.h"

#include "../render/renderState.h"
#include "../render/shaderFlag.h"
#include "../render/view.h"
#include "../render/viewBuilder.h"

#include <cassert>

using namespace render;

/**
 * constructor
 *
 * @param builder
 * @param view
 * @param node
 */
ViewWrapper::ViewWrapper(Builder & builder,
		const std::shared_ptr<render::View> & view,
		const std::shared_ptr<SgNode> & node) :
		TaskWrapper(view), builder(builder), node(node) {
}

/**
 * destructor
 */
ViewWrapper::~ViewWrapper() {
}

/**
 *
 */
void ViewWrapper::init() {
	auto view = std::static_pointer_cast<View>(getRenderTask());

	ViewBuilder vb(view, builder.getLights().getLighting(view));
	auto & state = vb.getState();

	if (view->isType(View::Type::REGULAR)) {
		vb.addOccluders(builder.getOccluders());
		// add debug geometry
		vb.addDebugGeometry(builder.getDebugGeometry());
	}

	if (view->getModifiers()[View::Modifier::SNAPSHOT]) {
		state.addShaderFlag(ShaderFlag::valueOf("UNLIT"));
	}

	// traverse scene
	node->visualize(vb);

	builder.incPolyCount(vb.getPolyCount());

	// new tasks add during render pass
	for (const auto & v : vb.getAdditionalViewTasks()) {
		builder.addTask(std::make_shared<ViewWrapper>(builder, v, node));
	}
}
