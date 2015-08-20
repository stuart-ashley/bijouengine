#include "decal.h"

#include "texture.h"
#include "textureManager.h"

#include "../core/rect.h"

#include "../scripting/bool.h"
#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

using namespace render;

namespace {
	std::vector<BaseParameter> params = { Parameter<String>("name", nullptr),
			Parameter<String>("texture", nullptr), Parameter<Bool>("flip",
					Bool::False()), Parameter<Rect>("rect", nullptr) };
	/*
	 *
	 */
	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);
			const auto & name =
					std::static_pointer_cast<String>(args["name"])->getValue();
			const auto & filename = std::static_pointer_cast<String>(
					args["texture"])->getValue();
			bool flip = (args["flip"] == Bool::True());
			const auto & rect = std::static_pointer_cast<Rect>(args["rect"]);
			stack.emplace(
					std::make_shared<Decal>(name,
							TextureManager::getInstance().getImage(currentDir,
									filename, true, true, true), flip, *rect));
		}
	};
}

struct Decal::impl {
	std::string name;
	std::shared_ptr<Texture> texture;
	bool flip;
	Rect rect;

	impl(const std::string & name, const std::shared_ptr<Texture> & texture,
			bool flip, const Rect & rect) :
			name(name), texture(texture), flip(flip), rect(rect) {
	}

};

/**
 * constructor
 *
 * @param name     name of decal
 * @param texture  texture for decal
 * @param flip     flip vertically
 * @param rect     decal rectangle within texture
 */
Decal::Decal(const std::string & name, const std::shared_ptr<Texture> & texture,
		bool flip, const Rect & rect) :
		pimpl(new impl(name, texture, flip, rect)) {
}

/**
 * destructor
 */
Decal::~Decal(){
}

/*
 *
 */
bool Decal::getFlip() const {
	return pimpl->flip;
}

/*
 *
 */
const std::string & Decal::getName() const {
	return pimpl->name;
}

/*
 *
 */
const Rect & Decal::getRect() const {
	return pimpl->rect;
}

/*
 *
 */
const std::shared_ptr<Texture> & Decal::getTexture() const {
	return pimpl->texture;
}

/**
 * get script object factory for Decal
 *
 * @param currentDir  current directory of caller
 *
 * @return            Decal factory
 */
STATIC ScriptObjectPtr Decal::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
