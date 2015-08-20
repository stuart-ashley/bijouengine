#include "panelTask.h"

#include "builder.h"
#include "taskWrapper.h"
#include "updateState.h"

#include "../core/animation.h"
#include "../core/color.h"
#include "../core/config.h"
#include "../core/rect.h"

#include "../render/panel.h"
#include "../render/shaderManager.h"
#include "../render/shaderTag.h"
#include "../render/texture.h"
#include "../render/textureManager.h"
#include "../render/uniform.h"
#include "../render/uniformArray.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/none.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

using namespace render;

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("name", std::make_shared<String>("")),
			Parameter<Rect>("srcRect", std::make_shared<Rect>(0.f, 1.f, 0.f, 1.f)),
			Parameter<Rect>("dstRect", std::make_shared<Rect>(0.f, 1.f, 0.f, 1.f)),
			Parameter<Texture>("source", None::none()),
			Parameter<ShaderTag>("shader", None::none()),
			Parameter<Real>("level", std::make_shared<Real>(0)),
			Parameter<String>("blend", std::make_shared<String>("NONE")),
			Parameter<Real>("alpha", std::make_shared<Real>(1)),
			Parameter<Texture>("target", None::none()),
			Parameter<List>("uniforms", std::make_shared<List>()) };

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

			auto name =
					std::static_pointer_cast<String>(args["name"])->getValue();

			auto srcRect = *std::static_pointer_cast<Rect>(args["srcRect"]);

			auto dstRect = *std::static_pointer_cast<Rect>(args["dstRect"]);

			auto arg = args["shader"];
			auto shaderTag =
					(arg == None::none()) ?
							ShaderManager::getInstance().getImageTag() :
							*std::static_pointer_cast<ShaderTag>(arg);

			arg = args["target"];
			auto target =
					(arg == None::none()) ?
							TextureManager::getInstance().getScreen() :
							std::static_pointer_cast<Texture>(arg);

			arg = args["source"];
			auto source =
					(arg == None::none()) ?
							nullptr : std::static_pointer_cast<Texture>(arg);

			auto blendStr =
					std::static_pointer_cast<String>(args["blend"])->getValue();
			BlendFlag blend;
			if (blendStr == "NONE") {
				blend = BlendFlag::NONE;
			} else if (blendStr == "ADDITIVE") {
				blend = BlendFlag::ADDITIVE;
			} else if (blendStr == "SRC_ALPHA_ADDITIVE") {
				blend = BlendFlag::SRC_ALPHA_ADDITIVE;
			} else if (blendStr == "SRC_ALPHA") {
				blend = BlendFlag::SRC_ALPHA;
			} else {
				scriptExecutionAssert(false,
						"Require blend NONE, ADDITIVE, SRC_ALPHA_ADDITIVE or SRC_ALPHA");
			}

			auto level =
					std::static_pointer_cast<Real>(args["level"])->getInt32();

			auto color = Color::white();

			float alpha =
					std::static_pointer_cast<Real>(args["alpha"])->getFloat();

			UniformArray uniformArray;

			std::vector<Animation> animatedUniforms;
			for (const auto & e : *std::static_pointer_cast<List>(
					args["uniforms"])) {

				if (typeid(*e) == typeid(Uniform)) {
					uniformArray.add(*std::static_pointer_cast<Uniform>(e));
				} else if (typeid(*e) == typeid(Animation)) {
					animatedUniforms.emplace_back(
							*std::static_pointer_cast<Animation>(e));
				} else {
					scriptExecutionAssert(false, "Require Uniform list");
				}
			}
			stack.push(
					std::make_shared<PanelTask>(name, srcRect, dstRect, shaderTag,
							target, source, blend, level, color, alpha,
							uniformArray, animatedUniforms));
		}
	};

	/*
	 *
	 */
	class GetDstRect: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			auto panelTask = std::static_pointer_cast<PanelTask>(self);

			stack.push(std::make_shared<Rect>(panelTask->getDstRect()));
		}
	};

	/*
	 *
	 */
	class SetAlpha: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto panelTask = std::static_pointer_cast<PanelTask>(self);

			auto alpha = static_cast<float>(getNumericArg(stack, 1));

			panelTask->setAlpha(alpha);
		}
	};

	/*
	 *
	 */
	class SetBGColor: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto panelTask = std::static_pointer_cast<PanelTask>(self);

			auto color = getArg<Color>("Color", stack, 1);

			panelTask->setBGColor(color);
		}
	};

	/*
	 *
	 */
	class SetDstRect: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto panelTask = std::static_pointer_cast<PanelTask>(self);

			auto dstRect = getArg<Rect>("Rect", stack, 1);

			panelTask->setDstRect(dstRect);
		}
	};

	/*
	 *
	 */
	class SetOpaque: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto panelTask = std::static_pointer_cast<PanelTask>(self);

			auto opaque = getBoolArg(stack, 1);

			panelTask->setOpaque(opaque);
		}
	};

	/*
	 *
	 */
	class SetSrcRect: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto panelTask = std::static_pointer_cast<PanelTask>(self);

			auto srcRect = getArg<Rect>("Rect", stack, 1);

			panelTask->setSrcRect(srcRect);
		}
	};

	auto sourceUID = Uniform::getUID("Source");
}

struct PanelTask::impl {

	std::string name;
	std::vector<float> frames;
	Rect srcRect;
	Rect dstRect;
	ShaderTag shaderTag;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Texture> source;
	BlendFlag blend;
	int level;
	Color color;
	float alpha;
	UniformArray uniforms;
	std::vector<Animation> animatedUniforms;
	Color bgColor;
	bool opaque;
	bool valid;

	impl(const std::string & name, const Rect & sRect, const Rect & dRect,
			const ShaderTag & shader, const std::shared_ptr<Texture> & texture,
			const std::shared_ptr<Texture> & source, BlendFlag blend, int level,
			const Color & color, float a, const UniformArray & uniforms,
			const std::vector<Animation> & animatedUniforms) :
					name(name),
					frames(animatedUniforms.size(), 0),
					srcRect(sRect),
					dstRect(dRect),
					shaderTag(shader),
					texture(texture),
					source(source),
					blend(blend),
					level(level),
					color(color),
					alpha(a),
					uniforms(uniforms),
					animatedUniforms(animatedUniforms),
					bgColor(Color::black()),
					opaque(false),
					valid(false) {
	}
};

PanelTask::PanelTask(const std::string & name, const Rect & sRect,
		const Rect & dRect, const render::ShaderTag & shader,
		const std::shared_ptr<render::Texture> & texture,
		const std::shared_ptr<render::Texture> & source,
		render::BlendFlag blend, int level, const Color & color, float a,
		const render::UniformArray & uniforms,
		const std::vector<Animation> & animatedUniforms) :
				pimpl(new impl(name, sRect, dRect, shader, texture, source,
						blend, level, color, a, uniforms, animatedUniforms)) {
}

/**
 * destructor
 */
VIRTUAL PanelTask::~PanelTask() {
}

const Rect & PanelTask::getDstRect() const {
	return pimpl->dstRect;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr PanelTask::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getDstRect", std::make_shared<GetDstRect>() },
			{ "setAlpha", std::make_shared<SetAlpha>() },
			{ "setBGColor", std::make_shared<SetBGColor>() },
			{ "setDstRect", std::make_shared<SetDstRect>() },
			{ "setOpaque", std::make_shared<SetOpaque>() },
			{ "setSrcRect", std::make_shared<SetSrcRect>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

void PanelTask::setAlpha(float alpha) {
	pimpl->alpha = alpha;
}

void PanelTask::setBGColor(const Color & bgColor) {
	pimpl->bgColor = bgColor;
}

void PanelTask::setDstRect(const Rect & dstRect) {
	pimpl->dstRect = dstRect;
}

void PanelTask::setOpaque(bool opaque) {
	pimpl->opaque = opaque;
}

void PanelTask::setSrcRect(const Rect & srcRect) {
	pimpl->srcRect = srcRect;
}

/**
 *
 * @param builder
 */
OVERRIDE void PanelTask::taskInit(Builder & builder) {
	if (pimpl->valid == false) {
		return;
	}

	UniformArray uniformArray = pimpl->uniforms;
	if (pimpl->source != nullptr) {
		uniformArray.add(Uniform(sourceUID, pimpl->source));
	}

	for (size_t i = 0, n = pimpl->frames.size(); i < n; ++i) {
		uniformArray.add(Uniform(pimpl->animatedUniforms[i], pimpl->frames[i]));
	}

	auto shader = ShaderManager::getInstance().getShader(pimpl->shaderTag);

	auto panel = std::make_shared<Panel>(pimpl->name, pimpl->srcRect,
			pimpl->dstRect, uniformArray, shader, pimpl->texture, pimpl->source,
			pimpl->blend, pimpl->level, pimpl->color, pimpl->alpha,
			pimpl->opaque, pimpl->bgColor);

	builder.addTask(std::make_shared<TaskWrapper>(panel));
}

/**
 *
 */
OVERRIDE void PanelTask::update(UpdateState & state) {
	if (ShaderManager::getInstance().getShader(pimpl->shaderTag) == nullptr) {
		return;
	}

	float speed = Config::getInstance().getFloat("simulationSpeed");
	float dt = speed * state.getTimeStep();

	for (size_t i = 0, n = pimpl->frames.size(); i < n; ++i) {
		const auto & animation = pimpl->animatedUniforms.at(i);
		if (animation.validate() == false) {
			return;
		}

		float frame = pimpl->frames.at(i) + animation.getFPS() * dt;
		frame = std::max(frame, animation.getStartFrame());
		if (frame > animation.getEndFrame()) {
			frame = animation.getStartFrame();
		}
		pimpl->frames[i] = frame;
	}

	pimpl->valid = true;
}

/**
 * get script object factory for PanelTask
 *
 * @return            PanelTask factory
 */
STATIC ScriptObjectPtr PanelTask::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}

