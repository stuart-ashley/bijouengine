#include "sgEnvmap.h"

#include "builder.h"
#include "updateState.h"

#include "../core/aspect.h"
#include "../core/boundingBox.h"
#include "../core/color.h"
#include "../core/debugGeometry.h"
#include "../core/perspectiveCamera.h"

#include "../render/regularView.h"
#include "../render/renderState.h"
#include "../render/shaderManager.h"
#include "../render/texture.h"
#include "../render/textureManager.h"
#include "../render/uniform.h"
#include "../render/uniformArray.h"
#include "../render/view.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

#include <cmath>

using namespace render;

namespace {

	struct Factory: public Executable {

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			std::shared_ptr<Texture> texture = nullptr;
			std::vector<std::shared_ptr<UpdateNode>> updateNodes;
			std::vector<std::shared_ptr<TaskInitNode>> taskInitNodes;
			std::vector<std::shared_ptr<VisualizeNode>> visualizeNodes;

			for (unsigned i = 0; i < nArgs; ++i) {
				auto arg = stack.top();
				stack.pop();

				if (typeid(*arg) == typeid(Texture)) {
					scriptExecutionAssert(texture == nullptr,
							"Require single texture");
					texture = std::static_pointer_cast<Texture>(arg);
					continue;
				}

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
			scriptExecutionAssert(texture != nullptr, "Require texture");

			stack.push(
					std::make_shared<SgEnvmap>(texture, updateNodes,
							taskInitNodes, visualizeNodes));
		}
	};

	Texture::Face xyz[] = { Texture::Face::POSITIVE_X,
			Texture::Face::NEGATIVE_X, Texture::Face::POSITIVE_Y,
			Texture::Face::NEGATIVE_Y, Texture::Face::POSITIVE_Z,
			Texture::Face::NEGATIVE_Z, };
	const char * exts[] = { "+x", "-x", "+y", "-y", "+z", "-z" };

	Quat rotations[] = {
	/* looking in -x direction */
	Quat(std::sqrt(2.f) / 2.f, 0, -std::sqrt(2.f) / 2.f, 0),
	/* looking in +x direction */
	Quat(std::sqrt(2.f) / 2.f, 0, std::sqrt(2.f) / 2.f, 0),
	/* looking in -y direction */
	Quat(std::sqrt(2.f) / 2.f, 0, 0, std::sqrt(2.f) / 2.f),
	/* looking in +y direction */
	Quat(-std::sqrt(2.f) / 2.f, 0, 0, std::sqrt(2.f) / 2.f),
	/* looking in -z direction */
	Quat(1, 0, 0, 0),
	/* looking in +z direction */
	Quat(0, 0, 1, 0) };

	auto fadeEnvmapUID = Uniform::getUID("u_fadeEnvmap");
	auto cubeEnvmapUID = Uniform::getUID("u_cubeEnvmap");
	auto viewToEnvmapUID = Uniform::getUID("u_viewToEnvmap");
	auto envmapViewPositionUID = Uniform::getUID("u_envmapViewPosition");
}

struct SgEnvmap::impl {
	std::shared_ptr<Texture> cubemap;
	std::vector<std::shared_ptr<UpdateNode>> updateNodes;
	std::vector<std::shared_ptr<TaskInitNode>> taskInitNodes;
	std::vector<std::shared_ptr<VisualizeNode>> visualizeNodes;
	std::shared_ptr<View> views[6];

	impl(std::shared_ptr<Texture> & cubemap,
			const std::vector<std::shared_ptr<UpdateNode>>& updateNodes,
			const std::vector<std::shared_ptr<TaskInitNode>> & taskInitNodes,
			const std::vector<std::shared_ptr<VisualizeNode>> & visualizeNodes) :
					cubemap(cubemap),
					updateNodes(updateNodes),
					taskInitNodes(taskInitNodes),
					visualizeNodes(visualizeNodes) {
	}
};

/**
 * constructor
 *
 * @param cubemap
 * @param updateNodes
 * @param taskInitNodes
 * @param visualizeNodes
 */
SgEnvmap::SgEnvmap(std::shared_ptr<render::Texture> & cubemap,
		const std::vector<std::shared_ptr<UpdateNode>>& updateNodes,
		const std::vector<std::shared_ptr<TaskInitNode>> & taskInitNodes,
		const std::vector<std::shared_ptr<VisualizeNode>> & visualizeNodes) :
		pimpl(new impl(cubemap, updateNodes, taskInitNodes, visualizeNodes)) {
}

/**
 * destructor
 */
VIRTUAL SgEnvmap::~SgEnvmap() {
}

/**
 *
 * @param state
 */
OVERRIDE void SgEnvmap::update(UpdateState & state) {
	state.pushState();
	for (const auto & node : pimpl->updateNodes) {
		node->update(state);
	}
	state.popState();
}

/**
 *
 * @param builder
 */
OVERRIDE void SgEnvmap::taskInit(Builder & builder) {
	for (int i = 0; i < 6; ++i) {
		pimpl->views[i] = nullptr;
	}
	builder.pushState();
	for (const auto & node : pimpl->taskInitNodes) {
		node->taskInit(builder);
	}
	builder.popState();
}

/**
 *
 * @param vb
 */
OVERRIDE void SgEnvmap::visualize(render::ViewBuilder & vb) {
	// skip nodes when rendering cubemap
	for (const auto & view : pimpl->views) {
		if (vb.getView() == view) {
			return;
		}
	}

	const auto & name = TextureManager::getInstance().getName(
			pimpl->cubemap->getId());

	// create cubemap views
	for (int i = 0; i < 6; ++i) {
		if (pimpl->views[i] == nullptr) {
			auto camera = std::make_shared<PerspectiveCamera>(name + exts[i],
					90.f, 1.f, .01f, 1000.f);
			const auto & tex = TextureManager::getInstance().getCubeMap(name,
					pimpl->cubemap->getWidth(), pimpl->cubemap->getHeight(),
					xyz[i]);

			auto t = vb.getState().getTransform();
			t.rotate(rotations[i]);

			Aspect aspect(camera, t);

			UniformArray uniforms;

			View::ModifierSet modifiers;
			modifiers[View::Modifier::LORES] = true;

			pimpl->views[i] = std::make_shared<RegularView>(camera->getName(),
					aspect, tex, Rect(0, 1, 0, 1), uniforms,
					ShaderManager::getInstance().getUberTag(), View::Cull::BACK,
					View::DepthCompare::LESS, modifiers, 0);

			vb.addView(pimpl->views[i]);
		}
	}

	// distance from camera
	const auto & envmapTransform = vb.getState().getTransform();
	double d = vb.getView()->getLodAspect().getRotTrans().distanceTo(
			envmapTransform);

	if (d < 50 && vb.getView()->isType(View::Type::REGULAR)) {
		const auto & viewTransform = vb.getView()->getAspect().getRotTrans();

		// rotate from view to envmap space
		Transform viewToEnvmap = viewTransform.to(envmapTransform);
		Mat3 viewToEnvmapRotation = viewToEnvmap.getRotationMatrix();

		// envmap position in view space
		auto envmapViewPosition = envmapTransform.getTranslation();
		viewTransform.inverseTransformPoint(envmapViewPosition);

		UniformArray uniforms;
		uniforms.add(Uniform(fadeEnvmapUID, (float) (1 - d / 50)));
		uniforms.add(Uniform(cubeEnvmapUID, pimpl->cubemap));
		uniforms.add(Uniform(viewToEnvmapUID, viewToEnvmapRotation));
		uniforms.add(Uniform(envmapViewPositionUID, envmapViewPosition));

		vb.getState().addUniforms(uniforms);
	} else {
		ShaderFlags flags;
		flags[ShaderFlag::valueOf("IGNORE_ENVMAP")] = true;

		vb.getState().addShaderFlags(flags);
	}

	vb.pushState();
	for (const auto & node : pimpl->visualizeNodes) {
		node->visualize(vb);
	}
	vb.popState();
}

/**
 * get script object factory for SgEnvmap
 *
 * @return  SgEnvmap factory
 */
STATIC const ScriptObjectPtr & SgEnvmap::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
