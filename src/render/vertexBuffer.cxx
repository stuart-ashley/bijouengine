#include "vertexBuffer.h"

#include "glDebug.h"

#include "../core/binaryFileCache.h"

#include "../scripting/executable.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <cassert>
#include <mutex>
#include <vector>

#include <GL/glew.h>

using namespace render;

namespace {

	std::vector<BaseParameter> params = { Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr), Parameter<Real>("byteCount",
					nullptr) };
	/*
	 *
	 */
	class Factory: public Executable {
	public:
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);
			const auto & filename = std::static_pointer_cast<String>(
					args["file"])->getValue();
			int offset =
					std::static_pointer_cast<Real>(args["offset"])->getInt32();
			int byteCount =
					std::static_pointer_cast<Real>(args["byteCount"])->getInt32();
			Binary binary("", BinaryFileCache::get(currentDir, filename),
					offset, byteCount);
			stack.push(std::make_shared<VertexBuffer>(binary));
		}
	};
}

/**
 *
 * @param binary
 */
VertexBuffer::VertexBuffer(const Binary & binary) :
		binary(binary), vbo(0) {
}

/**
 *
 */
void VertexBuffer::bind() {
	validate();

	if (vbo > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		return;
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, binary.getByteCount(), binary.getData(),
			GL_STATIC_DRAW);

	GlDebug::printOpenGLError();
}

/**
 *
 */
bool VertexBuffer::validate() {
	// already validated if vbo setup
	if (vbo > 0) {
		return true;
	}

	// validate maybe called from update or render thread, hence lock
	std::lock_guard<std::mutex> locker(lock);

	return binary.valid();
}

/**
 * get script object factory for VertexBuffer
 *
 * @param currentDir  current directory of caller
 *
 * @return            VertexBuffer factory
 */
STATIC ScriptObjectPtr VertexBuffer::getFactory(
		const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}

