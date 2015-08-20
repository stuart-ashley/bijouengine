#include "sgEdgeIndexArray.h"

#include "../core/binaryFileCache.h"

#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"
#include "../scripting/string.h"

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr),
			Parameter<Real>("indexCount", nullptr) };
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

			auto file = std::static_pointer_cast<String>(args["file"]);
			auto offset = std::static_pointer_cast<Real>(args["offset"]);
			auto count = std::static_pointer_cast<Real>(args["indexCount"]);

			scriptExecutionAssert(offset->isInt32(), "Require integer offset");

			scriptExecutionAssert(count->isInt32(),
					"Require integer indexCount");

			stack.push(
					std::make_shared<SgEdgeIndexArray>(
							IndexArray(
									BinaryFileCache::get(currentDir,
											file->getValue()),
									offset->getInt32(), count->getInt32())));
		}
	};
}

/**
 * Construct from index array
 *
 * @param array  array of edge indices
 */
SgEdgeIndexArray::SgEdgeIndexArray(const IndexArray & array) :
		m_array(array) {
}

/**
 * Add index array to render graph
 */
OVERRIDE void SgEdgeIndexArray::visualize(render::ViewBuilder & vb) {
	if (m_array.validate()) {
		vb.addEdgeIndexArray(m_array);
	}
}

/**
 * get script object factory for SgEdgeIndexArray
 *
 * @param currentDir  current directory of caller
 *
 * @return            SgEdgeIndexArray factory
 */
STATIC ScriptObjectPtr SgEdgeIndexArray::getFactory(
		const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
