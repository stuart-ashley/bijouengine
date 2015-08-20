#include "font.h"

#include "decal.h"
#include "shaderTag.h"

#include "../core/loadingCallback.h"
#include "../core/loadManager.h"
#include "../core/loadManagerUtils.h"
#include "../core/rect.h"

#include "../scripting/list.h"
#include "../scripting/program.h"
#include "../scripting/scriptExecutionState.h"

#include <cassert>
#include <iostream>
#include <unordered_map>

using namespace render;

struct Font::impl: public LoadingCallback {
	std::shared_ptr<ShaderTag> shader;
	std::unordered_map<char, std::shared_ptr<Decal>> decals;

	impl(const std::string & currentDir, const std::string & name) :
			LoadingCallback(currentDir, name) {
		LoadManager::getInstance()->load(*this);
	}

	/*
	 *
	 */
	void load(std::istream & stream) {
		auto prog = ::Program::create(getCanonicalFilename(), stream);

		auto currentDir = LoadManagerUtils::getDirectory(
				getCanonicalFilename());

		prog->setMember("Rect", Rect::getFactory());
		prog->setMember("Decal", Decal::getFactory(currentDir));
		prog->setMember("Shader", ShaderTag::getFactory(currentDir));

		ScriptExecutionState execState;

		prog->init(execState);

		shader = std::static_pointer_cast<ShaderTag>(
				prog->getMember(execState, "shader"));

		auto list = std::static_pointer_cast<List>(
				prog->getMember(execState, "decals"));

		for (const auto & e : *list) {
			auto decal = std::static_pointer_cast<Decal>(e);
			auto result = decals.emplace(decal->getName()[0], decal);
			assert(result.second);
		}
	}
};

/*
 *
 */
Font::Font(const std::string & currentDir, const std::string & name) :
		pimpl(new impl(currentDir, name)) {
}

/*
 *
 */
Font::~Font(){
}

/*
 *
 */
const std::shared_ptr<Decal> & Font::getDecal(char c) const {
	auto it = pimpl->decals.find(c);
	if (it == pimpl->decals.end()) {
		std::cerr << "Can't find decal '" << c << "'" << std::endl;
		assert(false);
	}
	return it->second;
}

/*
 *
 */
const ShaderTag & Font::getShader() const {
	return *pimpl->shader;
}

/*
 *
 */
bool Font::validate() const {
	return pimpl->shader != nullptr;
}
