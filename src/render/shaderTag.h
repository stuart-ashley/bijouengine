#pragma once

#include "program.h"
#include "shaderFlag.h"

#include "../scripting/scriptObject.h"

#include <memory>
#include <vector>

namespace render {

	class ShaderTag: public ScriptObject {
	public:

		ShaderTag(const std::shared_ptr<Program> & vp,
				const std::shared_ptr<Program> & gp,
				const std::shared_ptr<Program> & fp);

		ShaderTag(const std::shared_ptr<Program> & vp,
				const std::shared_ptr<Program> & gp,
				const std::shared_ptr<Program> & fp, const ShaderFlags & flags);

		ShaderTag(const ShaderTag & other);

		~ShaderTag();

		void addFlag(size_t flag);

		void addFlags(const ShaderFlags & additionalFlags);

		ShaderFlags getActiveFlags() const;

		const std::shared_ptr<Program> & getFragmentProgram() const;

		const std::shared_ptr<Program> & getGeometryProgram() const;

		size_t getHash() const;

		const std::shared_ptr<Program> & getVertexProgram() const;

		bool hasFlag(size_t flag) const;

		bool operator==(const ShaderTag & other) const;

		void overlay(const ShaderTag & overlay);

		void removeFlag(size_t flag);

		std::string toString() const;

		bool validate() const;

		/**
		 * get script object factory for ShaderTag
		 *
		 * @param currentDir  current directory of caller
		 *
		 * @return            ShaderTag factory
		 */
		static ScriptObjectPtr getFactory(const std::string & currentDir);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

namespace std {
	template<>
	struct hash<render::ShaderTag> {
		std::size_t operator()(const render::ShaderTag & key) const {
			return key.getHash();
		}
	};
}

