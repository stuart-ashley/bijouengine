#include "viewTask.h"

#include "builder.h"
#include "sgCamera.h"
#include "sgNode.h"
#include "updateState.h"
#include "viewWrapper.h"

#include "../core/aspect.h"
#include "../core/color.h"
#include "../core/perspectiveCamera.h"
#include "../core/rect.h"
#include "../core/vec2.h"

#include "../render/regularView.h"
#include "../render/shaderManager.h"
#include "../render/shaderTag.h"
#include "../render/shadowView.h"
#include "../render/textureManager.h"
#include "../render/uniformArray.h"
#include "../render/view.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/none.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <algorithm>
#include <cassert>
#include <iostream>

using namespace render;

namespace {

	std::vector<BaseParameter> params = {
			Parameter<String>("name", std::make_shared<String>("")),
			Parameter<SgCamera>("camera", nullptr),
			Parameter<List>("targets", None::none()),
			Parameter<Rect>("rect", std::make_shared<Rect>(0.f, 1.f, 0.f, 1.f)),
			Parameter<String>("type", std::make_shared<String>("REGULAR")),
			Parameter<String>("cull", std::make_shared<String>("BACK")),
			Parameter<String>("depthCompare", std::make_shared<String>("LESS")),
			Parameter<List>("uniforms", std::make_shared<List>()),
			Parameter<Shader>("shader", None::none()),
			Parameter<SgNode>("root", nullptr) };

	ShaderTag getShaderTag(const ScriptObjectPtr & e, View::Type type) {
		if (e != None::none()) {
			return *std::static_pointer_cast<ShaderTag>(e);
		} else {
			switch (type) {
			case View::Type::REGULAR:
				return ShaderManager::getInstance().getUberTag();
			case View::Type::SHADOW:
				return ShaderManager::getInstance().getShadowTag();
			default:
				throw ScriptExecutionException("Unknown view type");
			}
		}
	}

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

			auto camera = std::static_pointer_cast<SgCamera>(args["camera"]);

			std::vector<std::shared_ptr<Texture>> targets;
			auto e = args["targets"];
			if (e == None::none()) {
				targets.emplace_back(TextureManager::getInstance().getScreen());
			} else {
				for (const auto & e : *std::static_pointer_cast<List>(
						args["targets"])) {
					scriptExecutionAssertType<Texture>(e,
							"Require list of textures");
					targets.emplace_back(std::static_pointer_cast<Texture>(e));
				}
			}

			auto rect = *std::static_pointer_cast<Rect>(args["rect"]);

			View::Cull cull;
			const auto & cullStr = std::static_pointer_cast<String>(
					args["cull"])->getValue();
			if (cullStr == "BACK") {
				cull = View::Cull::BACK;
			} else if (cullStr == "FRONT") {
				cull = View::Cull::FRONT;
			} else if (cullStr == "FRONTBACK") {
				cull = View::Cull::FRONTBACK;
			} else if (cullStr == "NONE") {
				cull = View::Cull::NONE;
			} else {
				scriptExecutionAssert(false,
						"Unknown cull type: '" + cullStr
								+ "', should be one of BACK, FRONT, FRONTBACK, NONE");
			}

			View::DepthCompare depthCompare;
			const auto & depthCompareStr = std::static_pointer_cast<String>(
					args["depthCompare"])->getValue();
			if (depthCompareStr == "GREATER") {
				depthCompare = View::DepthCompare::GREATER;
			} else if (depthCompareStr == "LESS") {
				depthCompare = View::DepthCompare::LESS;
			} else {
				scriptExecutionAssert(false,
						"Unknown depth compare type: '" + depthCompareStr
								+ "', should be one of GREATER, LESS");
			}

			View::Type type;
			const auto & typeStr = std::static_pointer_cast<String>(
					args["type"])->getValue();
			if (typeStr == "REGULAR") {
				type = View::Type::REGULAR;
			} else if (typeStr == "SHADOW") {
				type = View::Type::SHADOW;
			} else {
				scriptExecutionAssert(false,
						"Unknown view type: '" + typeStr
								+ "', should be one of REGULAR, SHADOW");
			}

			UniformArray uniforms;
			for (const auto & e : *std::static_pointer_cast<List>(
					args["uniforms"])) {
				scriptExecutionAssertType<Uniform>(e,
						"Require list of uniforms");
				uniforms.add(*std::static_pointer_cast<Uniform>(e));
			}

			ShaderTag shader = getShaderTag(args["shader"], type);

			View::ModifierSet modifiers;
			int level = 0;
			const auto & bgColor = Color::black();
			auto root = std::static_pointer_cast<SgNode>(args["root"]);

			stack.push(
					std::make_shared<ViewTask>(name, camera, targets, rect,
							uniforms, shader, cull, depthCompare, type,
							modifiers, level, bgColor, root));
		}
	};

	struct GetCamera: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto viewTask = std::static_pointer_cast<ViewTask>(self);

			stack.push(viewTask->getCamera());
		}
	};

	struct GetRect: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto viewTask = std::static_pointer_cast<ViewTask>(self);

			stack.push(std::make_shared<Rect>(viewTask->getRect()));
		}
	};

	struct GetWorldRay: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto viewTask = std::static_pointer_cast<ViewTask>(self);

			auto pos = getArg<Vec2>("vec2", stack, 1);
			auto ray = viewTask->calculateRay(pos);
			stack.push(std::make_shared<Ray>(ray));
		}
	};

	struct SetCamera: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto viewTask = std::static_pointer_cast<ViewTask>(self);

			auto arg = stack.top();
			stack.pop();

			scriptExecutionAssertType<SgCamera>(arg,
					"Require SgCamera for argument 1");

			viewTask->setCamera(std::static_pointer_cast<SgCamera>(arg));
		}
	};
}

struct ViewTask::impl {
	std::string name;
	std::shared_ptr<SgCamera> camera;
	std::vector<std::shared_ptr<Texture>> textures;
	Rect rect;
	UniformArray uniforms;
	ShaderTag shader;
	View::Cull cull;
	View::DepthCompare depthCompare;
	View::Type type;
	View::ModifierSet modifiers;
	int level;
	Color bgColor;
	std::shared_ptr<SgNode> root;

	impl(const std::string & name, const std::shared_ptr<SgCamera> & camera,
			const std::vector<std::shared_ptr<Texture>> & textures,
			const Rect & rect, const UniformArray & uniforms,
			const ShaderTag & shader, View::Cull cull,
			View::DepthCompare depthCompare, View::Type type,
			const View::ModifierSet & modifiers, int level,
			const Color & bgColor, const std::shared_ptr<SgNode> & root) :
					name(name),
					camera(camera),
					textures(textures),
					rect(rect),
					uniforms(uniforms),
					shader(shader),
					cull(cull),
					depthCompare(depthCompare),
					type(type),
					modifiers(modifiers),
					level(level),
					bgColor(bgColor),
					root(root) {
	}
};

/**
 * constructor
 *
 * @param name
 * @param camera
 * @param textures
 * @param rect
 * @param uniforms
 * @param shader
 * @param cull
 * @param depthCompare
 * @param type
 * @param modifiers
 * @param level
 * @param bgColor
 * @param root
 */
ViewTask::ViewTask(const std::string & name,
		const std::shared_ptr<SgCamera> & camera,
		const std::vector<std::shared_ptr<render::Texture>> & textures,
		const Rect & rect, const render::UniformArray & uniforms,
		const render::ShaderTag & shader, render::View::Cull cull,
		render::View::DepthCompare depthCompare, render::View::Type type,
		const render::View::ModifierSet & modifiers, int level,
		const Color & bgColor, const std::shared_ptr<SgNode> & root) :
				pimpl(
						new impl(name, camera, textures, rect, uniforms, shader,
								cull, depthCompare, type, modifiers, level,
								bgColor, root)) {
}

/**
 * destructor
 */
ViewTask::~ViewTask() {
}

/**
 *
 * @param p
 * @return
 */
Ray ViewTask::calculateRay(const Vec2 & p) const {
	// calc x clamp at edges
	float x = (p.getX() - pimpl->rect.getLeft()) / pimpl->rect.getWidth();
	x = std::min(std::max(x, 0.f), 1.f);

	// calc y clamp at edges
	float y = (p.getY() - pimpl->rect.getBottom()) / pimpl->rect.getHeight();
	y = std::min(std::max(y, 0.f), 1.f);

	// normalized position
	Vec2 pos(x, y);

	// camera space ray
	auto ray = pimpl->camera->getCamera()->getRay(pos);

	// transform to world
	ray.transform(pimpl->camera->getAspect()->getRotTrans());

	return ray;
}

/**
 *
 * @return
 */
const std::shared_ptr<SgCamera> & ViewTask::getCamera() const {
	return pimpl->camera;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr ViewTask::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = { {
			"getCamera", std::make_shared<GetCamera>() }, { "getRect",
			std::make_shared<GetRect>() }, { "getWorldRay", std::make_shared<
			GetWorldRay>() }, { "setCamera", std::make_shared<SetCamera>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 * @return
 */
const Rect & ViewTask::getRect() const {
	return pimpl->rect;
}

/**
 *
 * @param camera
 */
void ViewTask::setCamera(const std::shared_ptr<SgCamera> & camera) const {
	pimpl->camera = camera;
}

/**
 *
 * @param builder
 */
OVERRIDE void ViewTask::taskInit(Builder & builder) {
	if (pimpl->root != nullptr) {
		builder.pushState();
		// undo any transform
		Transform t = builder.getTransform();
		builder.transform(t.inverse());
		pimpl->root->taskInit(builder);
		builder.popState();
	}

	const auto & aspect = pimpl->camera->getAspect();
	assert(aspect != nullptr);

	switch (pimpl->type) {
	case View::Type::SHADOW: {
		assert(pimpl->textures.size() == 1);

		auto view = std::make_shared<ShadowView>(pimpl->name, *aspect, *aspect,
				pimpl->textures[0], pimpl->rect, pimpl->uniforms, pimpl->shader,
				pimpl->cull, pimpl->depthCompare, pimpl->modifiers);
		builder.addTask(
				std::make_shared<ViewWrapper>(builder, view, pimpl->root));
		break;
	}
	case View::Type::REGULAR: {
		auto view = std::make_shared<RegularView>(pimpl->name, *aspect,
				pimpl->textures, pimpl->rect, pimpl->uniforms, pimpl->shader,
				pimpl->cull, pimpl->depthCompare, pimpl->modifiers,
				pimpl->level);
		builder.addTask(
				std::make_shared<ViewWrapper>(builder, view, pimpl->root));
		view->setBGColor(pimpl->bgColor);
		break;
	}
	default:
		std::cerr << "Unrecognised view type" << std::endl;
		assert(false);
	}
}

/**
 *
 * @param state
 */
OVERRIDE void ViewTask::update(UpdateState & state) {
	if (pimpl->root != nullptr) {
		state.pushState();
		// undo any transform
		Transform t = state.getTransform();
		state.transform(t.inverse());
		pimpl->root->update(state);
		state.popState();
	}
}

/**
 * get script object factory for ViewTask
 *
 * @return  ViewTask factory
 */
STATIC const ScriptObjectPtr & ViewTask::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
