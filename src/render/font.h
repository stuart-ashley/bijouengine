#pragma once

#include "../scripting/scriptObject.h"

#include <memory>

namespace render {
	class Decal;
	class ShaderTag;

	class Font: public ScriptObject {
	public:
		Font(const std::string & currentDir, const std::string & name);

		~Font();

		const std::shared_ptr<Decal> & getDecal(char c) const;

		const ShaderTag & getShader() const;

		bool validate() const;
	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

