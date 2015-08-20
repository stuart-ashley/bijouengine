#include "sgCollision.h"

#include "updateState.h"

#include "../core/collisionHierarchy.h"
#include "../core/debugGeometry.h"
#include "../core/transform.h"

#include "../render/renderState.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

namespace {
	/*
	 *
	 */
	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 2);

			auto name = getArg<String>("string", stack, 1).getValue();
			auto collision = getArg<CollisionHierarchy>("collision", stack, 2);

			stack.push(std::make_shared<SgCollision>(name, collision));
		}
	};
}

struct SgCollision::impl {
	std::string name;
	CollisionHierarchy collision;
	Transform transform;
	bool first;
	bool valid;

	impl(const std::string & name, const CollisionHierarchy & collision) :
			name(name), collision(collision), first(true), valid(false) {
	}
};

/**
 * constructor
 *
 * @param name
 * @param collision
 */
SgCollision::SgCollision(const std::string & name,
		const CollisionHierarchy & collision) :
		pimpl(new impl(name, collision)) {
}

/**
 * destructor
 */
SgCollision::~SgCollision() {
}

/**
 *
 * @param state
 */
OVERRIDE void SgCollision::update(UpdateState & state) {
	auto newTransform = state.getTransform();
	if (pimpl->first) {
		pimpl->first = false;
		pimpl->transform = newTransform;
	}
	pimpl->valid = pimpl->collision.validate();
	if (pimpl->valid) {
		state.addCollision(pimpl->name, pimpl->transform, newTransform,
				pimpl->collision);
		pimpl->transform = newTransform;
	}
}

/**
 *
 * @param vb
 */
OVERRIDE void SgCollision::visualize(render::ViewBuilder & vb) {
	if (pimpl->valid && vb.showCollisions()) {
		std::vector<DebugGeometry> debug;
		pimpl->collision.draw(debug, vb.getState().getTransform());
		vb.addDebugGeometry(debug);
	}
}

/**
 * get script object factory for SgCollision
 *
 * @return  SgCollision factory
 */
STATIC const ScriptObjectPtr & SgCollision::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
