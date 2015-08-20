#include "fontManager.h"

#include "font.h"

#include "../core/loadManager.h"

#include <unordered_map>

using namespace render;

namespace {
	std::unordered_map<std::string, std::shared_ptr<Font>> fontMap;
}

void FontManager::init() {
	getFont("", "default.font");
}

std::shared_ptr<Font> FontManager::getFont(const std::string & currentDir,
		const std::string & filename) {
	const std::string & canonicalFilename = LoadManager::getInstance()->getName(
			currentDir, filename);
	auto it = fontMap.find(canonicalFilename);
	if (it == fontMap.end()) {
		auto font = std::make_shared<Font>(currentDir, filename);
		fontMap.emplace(canonicalFilename, font);
		return font;
	} else {
		return it->second;
	}
}
