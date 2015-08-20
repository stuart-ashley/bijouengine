#pragma once

#include "shaderFlag.h"

#include <memory>
#include <vector>

namespace render {

	class Program {
	public:
		Program(const std::string & currentDir, const std::string & filename);
		~Program();

		const std::string & getName() const;

		const std::string & getSource() const;

		const ShaderFlags & getValidFlags() const;

		bool isLoaded() const;
	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

