#pragma once

#include <memory>
#include <string>
#include <vector>

namespace render {
	class ShaderUniforms {
	public:
		ShaderUniforms(int id);

		~ShaderUniforms();

		int getUniformLocation(size_t uid, int type) const;

		int getTextureId(size_t uid) const;

		int getNumUniforms() const;

		int getNumUserUniforms() const;

		const std::vector<std::string> & getUserUniformNames() const;

		void bindTextureIds() const;

		void dump() const;
	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}
