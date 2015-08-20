#pragma once

#include "../core/binary.h"

#include "../scripting/scriptObject.h"

#include <mutex>
#include <vector>

namespace render {
	/**
	 * VertexBuffer deletes copy functions removing the possibility of multiple
	 * vertex buffer allocations
	 */
	class VertexBuffer final: public ScriptObject {
	public:

		/**
		 *
		 * @param binary
		 */
		VertexBuffer(const Binary & binary);

		/**
		 * default destructor
		 */
		inline virtual ~VertexBuffer() = default;

		/**
		 *
		 */
		void bind();

		/**
		 *
		 */
		bool validate();

		/**
		 * get script object factory for VertexBuffer
		 *
		 * @param currentDir  current directory of caller
		 *
		 * @return            VertexBuffer factory
		 */
		static ScriptObjectPtr getFactory(const std::string & currentDir);

	private:
		VertexBuffer() = delete;
		VertexBuffer(const VertexBuffer &) = delete;
		VertexBuffer(const VertexBuffer &&) = delete;
		VertexBuffer & operator=(const VertexBuffer &) = delete;
		VertexBuffer & operator=(const VertexBuffer &&) = delete;

		Binary binary;
		unsigned vbo;
		std::mutex lock;
	};
}

