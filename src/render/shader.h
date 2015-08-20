#pragma once

#include "program.h"

#include <memory>
#include <string>
#include <vector>

namespace render {
	class Shader {
	public:

		Shader(int sid, const std::string & defs,
				const std::shared_ptr<Program> & vp,
				const std::shared_ptr<Program> & gp,
				const std::shared_ptr<Program> & fp, int maxInstances);

		~Shader();

		/**
		 *
		 */
		void bind();

		/**
		 *
		 */
		void bindEmpty();

		/**
		 *
		 */
		void build();

		/**
		 *
		 * @return  true if shader has been built, false otherwise
		 */
		bool built() const;

		/**
		 *
		 */
		void dump() const;

		/**
		 *
		 * @param name
		 * @return
		 */
		int getAttributeIndex(const std::string & name) const;

		/**
		 *
		 * @return
		 */
		unsigned getId() const;

		/**
		 *
		 * @return
		 */
		int getMaxInstances() const;

		/**
		 *
		 * @return
		 */
		int getNumAttributes() const;

		/**
		 *
		 * @return
		 */
		int getNumUniforms() const;

		/**
		 *
		 * @return
		 */
		int getNumUserUniforms() const;

		/**
		 *
		 * @param uid
		 * @return
		 */
		size_t getTextureId(size_t uid) const;

		/**
		 *
		 * @param uid
		 * @param type
		 * @return
		 */
		int getUniformLocation(size_t uid, int type) const;

		/**
		 *
		 * @return
		 */
		const std::vector<std::string> & getUserUniformNames() const;

		/**
		 * Disable vertex attributes and vertex buffer object
		 */
		void unbind();

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

