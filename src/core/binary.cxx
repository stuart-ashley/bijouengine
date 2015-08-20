#include "binary.h"

#include "binaryFileCache.h"

#include "../scripting/executable.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

#include <memory>
#include <vector>

namespace {
	std::vector<BaseParameter> params = {
			Parameter<String>("name", nullptr),
			Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr),
			Parameter<Real>("count", nullptr) };

	struct Factory: public Executable {
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			const auto & name =
					std::static_pointer_cast<String>(args["name"])->getValue();
			const auto & filename = std::static_pointer_cast<String>(
					args["file"])->getValue();
			int offset =
					std::static_pointer_cast<Real>(args["offset"])->getInt32();
			int count =
					std::static_pointer_cast<Real>(args["count"])->getInt32();
			stack.emplace(
					std::make_shared<Binary>(name,
							BinaryFileCache::get(currentDir, filename), offset,
							count));
		}
	};
}

/**
 * constructor
 *
 * @param name
 * @param binaryFile
 * @param offset
 * @param count
 */
Binary::Binary(const std::string & name,
		const std::shared_ptr<BinaryFile> & binaryFile, int offset, int count) :
		name(name), offset(offset), count(count), binaryFile(binaryFile) {
}

/**
 * get script object factory for Binary
 *
 * @param currentDir  current directory of caller
 *
 * @return            Binary factory
 */
STATIC ScriptObjectPtr Binary::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
