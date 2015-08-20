#pragma once

#include "shader.h"
#include "shaderTag.h"

#include <memory>

namespace render {
	class ShaderManager {
	public:

		ShaderManager();

		void buildShaders();

		const std::shared_ptr<Shader> & getDebugLinesShader() const;

		const std::shared_ptr<Shader> & getDebugShader() const;

		const std::shared_ptr<Shader> & getHorizontalBlurShader() const;

		const ShaderTag & getImageTag() const;

		std::shared_ptr<Program> getProgram(const std::string & currentDir,
				const std::string & name);

		std::shared_ptr<Shader> getShader(unsigned id);

		std::shared_ptr<Shader> getShader(const ShaderTag & tag);

		const ShaderTag & getShadowTag() const;

		const ShaderTag & getUberTag() const;

		const std::shared_ptr<Shader> & getVerticalBlurShader() const;

		static ShaderManager & getInstance();

		static void init();

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}
