#include "labelTask.h"

#include "builder.h"
#include "taskWrapper.h"

#include "../core/color.h"
#include "../core/rect.h"

#include "../render/font.h"
#include "../render/fontManager.h"
#include "../render/label.h"
#include "../render/texture.h"
#include "../render/textureManager.h"

#include "../scripting/executable.h"
#include "../scripting/none.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/string.h"

using namespace render;

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("font", std::make_shared<String>("default.font")),
			Parameter<Real>("level", std::make_shared<Real>(0)),
			Parameter<String>("justify", std::make_shared<String>("left")),
			Parameter<String>("text", std::make_shared<String>("")),
			Parameter<Bool>("opaque", Bool::False()),
			Parameter<Color>("bgColor", None::none()),
			Parameter<Color>("color", std::make_shared<Color>(1.f, 1.f, 1.f)),
			Parameter<Rect>("rect", std::make_shared<Rect>(0.f, 1.f, 0.f, 1.f)),
			Parameter<Texture>("texture", None::none()) };

	/*
	 *
	 */
	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			auto args = parameters.getArgs(nArgs, stack);
			auto fontArg = args["font"];
			auto levelArg = args["level"];
			auto justifyArg = args["justify"];
			auto leftArg = args["left"];
			auto textArg = args["text"];
			auto opaqueArg = args["opaque"];
			auto bgColorArg = args["bgColor"];
			auto colorArg = args["color"];
			auto rectArg = args["rect"];
			auto textureArg = args["texture"];

			auto font = FontManager::getFont(currentDir,
					std::static_pointer_cast<String>(fontArg)->getValue());

			scriptExecutionAssert(
					std::static_pointer_cast<Real>(levelArg)->isInt32(),
					"Require integer for level");

			int level = std::static_pointer_cast<Real>(levelArg)->getInt32();

			auto justify =
					std::static_pointer_cast<String>(justifyArg)->getValue();

			auto text = std::static_pointer_cast<String>(textArg)->getValue();

			auto opaque = std::static_pointer_cast<Bool>(opaqueArg)
					== Bool::True();

			auto bgColor = *std::static_pointer_cast<Color>(bgColorArg);

			auto color = *std::static_pointer_cast<Color>(colorArg);

			auto rect = *std::static_pointer_cast<Rect>(rectArg);

			auto texture =
					(textureArg == None::none()) ?
							TextureManager::getInstance().getScreen() :
							std::static_pointer_cast<Texture>(textureArg);

			stack.push(
					std::make_shared<LabelTask>("", font, rect, texture, opaque,
							color, bgColor, justify, text, level));
		}
	};

	/*
	 *
	 */
	class GetRect: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto & rect =
					std::static_pointer_cast<LabelTask>(self)->getRect();

			stack.push(std::make_shared<Rect>(rect));
		}
	};

	/*
	 *
	 */
	class SetBgColor: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto bgColor = getArg<Color>("color", stack, 1);

			std::static_pointer_cast<LabelTask>(self)->setBgColor(bgColor);
		}
	};

	/*
	 *
	 */
	class SetColor: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto color = getArg<Color>("color", stack, 1);

			std::static_pointer_cast<LabelTask>(self)->setColor(color);
		}
	};

	/*
	 *
	 */
	class SetOpaque: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto opaque = getBoolArg(stack, 1);

			std::static_pointer_cast<LabelTask>(self)->setOpaque(opaque);
		}
	};

	/*
	 *
	 */
	class SetRect: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto rect = getArg<Rect>("rectangle", stack, 1);

			std::static_pointer_cast<LabelTask>(self)->setRect(rect);
		}
	};

	/*
	 *
	 */
	class SetText: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 1);

			auto text = getArg<String>("string", stack, 1);

			std::static_pointer_cast<LabelTask>(self)->setText(text.getValue());
		}
	};

}

struct LabelTask::impl {
	std::string name;
	std::shared_ptr<Font> font;
	std::string justify;
	Rect rect;
	std::shared_ptr<render::Texture> texture;
	bool opaque;
	Color color;
	Color bgColor;
	std::string text;
	int level;

	impl(const std::string & name, const std::shared_ptr<Font> & font,
			const Rect & rect, const std::shared_ptr<render::Texture> & texture,
			bool opaque, const Color & color, const Color & bgColor,
			const std::string & justify, const std::string & text, int level) :
					name(name),
					font(font),
					justify(justify),
					rect(rect),
					texture(texture),
					opaque(opaque),
					color(color),
					bgColor(bgColor),
					text(text),
					level(level) {
	}
};

LabelTask::LabelTask(const std::string & name,
		const std::shared_ptr<render::Font> & font, const Rect & rect,
		const std::shared_ptr<render::Texture> & texture, bool opaque,
		const Color & color, const Color & bgColor, const std::string & justify,
		const std::string & text, int level) :
				pimpl( new impl(name, font, rect, texture, opaque, color,
						bgColor, justify, text, level)) {
}

/**
 * destructor
 */
VIRTUAL LabelTask::~LabelTask() {
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr LabelTask::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getRect", std::make_shared<GetRect>() },
			{ "setBgColor", std::make_shared<SetBgColor>() },
			{ "setColor", std::make_shared<SetColor>() },
			{ "setOpaque", std::make_shared<SetOpaque>() },
			{ "setRect", std::make_shared<SetRect>() },
			{ "setText", std::make_shared<SetText>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/*
 *
 */
const Rect & LabelTask::getRect() const {
	return pimpl->rect;
}

/*
 *
 */
void LabelTask::setBgColor(const Color & bgColor) {
	pimpl->bgColor = bgColor;
}

/*
 *
 */
void LabelTask::setColor(const Color & color) {
	pimpl->color = color;
}

/*
 *
 */
void LabelTask::setOpaque(bool opaque) {
	pimpl->opaque = opaque;
}

/*
 *
 */
void LabelTask::setRect(const Rect & rect) {
	pimpl->rect = rect;
}

/*
 *
 */
void LabelTask::setText(const std::string & text) {
	pimpl->text = text;
}

/*
 *
 */
OVERRIDE void LabelTask::taskInit(Builder & builder) {
	auto label = std::make_shared<Label>(pimpl->name, pimpl->font,
			pimpl->justify, pimpl->rect, pimpl->texture, pimpl->color,
			pimpl->opaque, pimpl->bgColor, pimpl->text, pimpl->level);

	builder.addTask(std::make_shared<TaskWrapper>(label));
}

/**
 * get script object factory for LabelTask
 *
 * @param currentDir  current directory of caller
 *
 * @return            LabelTask factory
 */
STATIC ScriptObjectPtr LabelTask::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
