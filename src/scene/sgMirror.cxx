#include "sgMirror.h"

#include "sgNode.h"

#include "../core/aspect.h"
#include "../core/normal.h"
#include "../core/plane.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include "../render/renderState.h"
#include "../render/uniform.h"
#include "../render/view.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/string.h"

#include <algorithm>
#include <unordered_set>

using namespace render;

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {

	Parameter<String>("name", nullptr),

	Parameter<Vec3>("point", nullptr),

	Parameter<Normal>("normal", nullptr),

	Parameter<Real>("reflect", nullptr),

	Parameter<Real>("refract", nullptr),

	Parameter<Real>("recursion", nullptr),

	Parameter<SgNode>("mesh", nullptr),

	};

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

			const auto & name =
					std::static_pointer_cast<String>(args["name"])->getValue();

			const auto & point = *std::static_pointer_cast<Vec3>(args["point"]);

			const auto & normal = *std::static_pointer_cast<Normal>(
					args["normal"]);

			float reflect =
					std::static_pointer_cast<Real>(args["reflect"])->getFloat();

			float refract =
					std::static_pointer_cast<Real>(args["refract"])->getFloat();

			size_t recursion =
					std::static_pointer_cast<Real>(args["recursion"])->getInt32();

			const auto & mesh = std::static_pointer_cast<SgNode>(args["mesh"]);

			stack.push(
					std::make_shared<SgMirror>(name, reflect, refract,
							Plane(point, normal), recursion, mesh));
		}
	};

	auto reflectUID = Uniform::getUID("u_reflect");
	auto refractUID = Uniform::getUID("u_refract");
}

struct SgMirror::impl {

	std::string name;
	float reflect;
	float refract;
	Plane plane;
	size_t recursion;
	ScriptObjectPtr mesh;
	/** ignore self reflection */
	std::unordered_set<std::string> ignore;

	impl(const std::string & name, float reflect, float refract,
			const Plane & plane, size_t recursion, const ScriptObjectPtr & mesh) :
					name(name),
					reflect(reflect),
					refract(refract),
					plane(plane),
					recursion(recursion),
					mesh(mesh) {
	}
};

/**
 * constructor
 *
 * @param name
 * @param reflect
 * @param refract
 * @param plane
 * @param recursion
 * @param mesh
 */
SgMirror::SgMirror(const std::string & name, float reflect, float refract,
		const Plane & plane, size_t recursion, const ScriptObjectPtr & mesh) :
		pimpl(new impl(name, reflect, refract, plane, recursion, mesh)) {
}

/**
 * destructor
 */
SgMirror::~SgMirror(){
}

/**
 *
 * @param builder
 */
OVERRIDE void SgMirror::taskInit(Builder & builder) {
	auto modelAsTask = std::dynamic_pointer_cast<TaskInitNode>(pimpl->mesh);
	if (modelAsTask != nullptr) {
		modelAsTask->taskInit(builder);
	}
	pimpl->ignore.clear();
}

/**
 *
 * @param state
 */
OVERRIDE void SgMirror::update(UpdateState & state) {
	auto modelAsUpdatable = std::dynamic_pointer_cast<UpdateNode>(pimpl->mesh);
	if (modelAsUpdatable != nullptr) {
		modelAsUpdatable->update(state);
	}
}

/**
 *
 * @param vb
 */
OVERRIDE void SgMirror::visualize(render::ViewBuilder & vb) {
	// world space plane
	auto worldPlane = pimpl->plane;
	worldPlane.transform(vb.getState().getTransform());

	// camera position in world space
	const auto & view = vb.getView();
	const auto & aspect = view->getAspect();
	auto m = aspect.getInverseTransform();
	Vec3 position = m.getTranslation();
	aspect.getRotTrans().transformPoint(position);

	// check mirror facing camera
	if (worldPlane.isBelow(position) == false) {
		return;
	}

	// stop self mirroring
	const auto & currentViewName = view->getName();
	if (pimpl->ignore.find(currentViewName) != pimpl->ignore.end()) {
		return;
	}

	vb.pushState();

	// limit recursion depth
	size_t n = std::count(currentViewName.begin(), currentViewName.end(), '\\');
	if (n <= pimpl->recursion + 1) {
		if (pimpl->reflect > 0) {
			auto newViewName = currentViewName + "\\" + pimpl->name;
			vb.applyMirror(newViewName, worldPlane);
			pimpl->ignore.emplace(newViewName);
		}

		if (pimpl->refract > 0) {
			worldPlane.flipNormal();

			auto viewName = currentViewName + "\\" + pimpl->name + ".refract";
			vb.applyRefraction(viewName, worldPlane);
			pimpl->ignore.emplace(viewName);
		}

		// reflection & refraction uniforms
		vb.getState().addUniform(Uniform(reflectUID, pimpl->reflect));
		vb.getState().addUniform(Uniform(refractUID, pimpl->refract));
	}

	auto modelAsVisual = std::dynamic_pointer_cast<VisualizeNode>(pimpl->mesh);
	if (modelAsVisual != nullptr) {
		modelAsVisual->visualize(vb);
	}

	vb.popState();
}

/**
 * get script object factory for SgMirror
 *
 * @return  SgMirror factory
 */
STATIC const ScriptObjectPtr & SgMirror::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
