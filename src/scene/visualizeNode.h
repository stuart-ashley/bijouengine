#pragma once

namespace render {
	class ViewBuilder;
}

class VisualizeNode {
public:
	inline virtual ~VisualizeNode() {
	}

	virtual void visualize(render::ViewBuilder & vb) = 0;
};

