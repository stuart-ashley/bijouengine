#pragma once

#include <memory>
#include <string>

namespace render {
	class ShaderAttributes {
	public:
		ShaderAttributes(int id);

		~ShaderAttributes();

		int getNumAttributes() const;

		int getAttributeIndex(const std::string & name) const;

		void enable() const;
		void disable() const;

		void dump() const;
	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

