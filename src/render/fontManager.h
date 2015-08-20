#pragma once

#include "font.h"

#include <memory>
#include <string>

class FontManager {
public:
	void init();
	static std::shared_ptr<render::Font> getFont(const std::string & currentDir,
			const std::string & filename);

};

