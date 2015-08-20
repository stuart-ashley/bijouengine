#pragma once

#include "builder.h"

#include "taskWrapper.h"

#include <memory>

namespace render {
	class View;
}

class ViewWrapper: public TaskWrapper {
public:

	/**
	 * constructor
	 *
	 * @param builder
	 * @param view
	 * @param node
	 */
	ViewWrapper(Builder & builder, const std::shared_ptr<render::View> & view,
			const std::shared_ptr<SgNode> & node);

	/**
	 * destructor
	 */
	~ViewWrapper();

	/**
	 *
	 */
	void init();

private:
	Builder & builder;
	std::shared_ptr<SgNode> node;
};

