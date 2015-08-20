#include "indexArray.h"

#include "binaryFile.h"
#include "binaryFileCache.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <atomic>
#include <mutex>

namespace {
	/*
	 *
	 */
	std::vector<BaseParameter> params = {
			Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr),
			Parameter<Real>("count", nullptr) };

	/*
	 *
	 */
	struct Factory: public Executable {
	public:
		std::string currentDir;
		Parameters parameters;

		Factory(const std::string & currentDir) :
				currentDir(currentDir), parameters(params) {
		}

		void execute(const std::shared_ptr<ScriptObject> &, unsigned nArgs,
				std::stack<std::shared_ptr<ScriptObject> > & stack) const
						override {
			if (typeid(*stack.top()) == typeid(List)) {
				auto list = getArg<List>("list", stack, 1);

				std::vector<int16_t> indices;
				indices.reserve(list.size());

				for (const auto & e : list) {
					scriptExecutionAssertType<Real>(e,
							"Require 16 bit integer list");

					auto num = std::static_pointer_cast<Real>(e);
					scriptExecutionAssert(num->isInt16(),
							"Require 16 bit integer list");

					indices.emplace_back(num->getInt16());
				}

				stack.emplace(std::make_shared<IndexArray>(indices));

				return;
			}
			auto args = parameters.getArgs(nArgs, stack);

			auto file = std::static_pointer_cast<String>(args["file"]);
			auto offset = std::static_pointer_cast<Real>(args["offset"]);
			auto count = std::static_pointer_cast<Real>(args["count"]);

			scriptExecutionAssert(offset->isInt32(), "Require integer offset");

			scriptExecutionAssert(count->isInt32(), "Require integer count");

			auto binFile = BinaryFileCache::get(currentDir, file->getValue());

			stack.emplace(
					std::make_shared<IndexArray>(binFile, offset->getInt32(),
							count->getInt32()));
		}
	}
	;
}

struct IndexArray::impl {
	std::vector<int16_t> buffer;
	std::shared_ptr<BinaryFile> binaryFile;
	int offset;
	size_t count;
	std::atomic<bool> m_valid;
	std::mutex m_lock;

	/*
	 *
	 */
	impl(const std::vector<int16_t> & array) :
			buffer(array), offset(0), count(array.size()), m_valid(true) {
	}

	/*
	 *
	 */
	impl(const std::shared_ptr<BinaryFile> & binaryFile, int offset, int count) :
			binaryFile(binaryFile), offset(offset), count(count), m_valid(false) {
	}

	/*
	 * maybe called from multiple threads
	 */
	bool validate() {
		if (m_valid) {
			return true;
		}

		std::lock_guard<std::mutex> locker(m_lock);

		if (m_valid == false && binaryFile->valid()) {
			buffer.reserve(count);

			auto indices =
					reinterpret_cast<const int16_t *>(binaryFile->getData()
							+ offset);

			buffer.insert(buffer.end(), indices, indices + count);

			m_valid = true;
		}
		return m_valid;
	}
};

/*
 *
 */
IndexArray::IndexArray(const std::initializer_list<int16_t> & list) :
		pimpl(std::make_shared<impl>(list)) {
}

/*
 *
 */
IndexArray::IndexArray(const std::vector<int16_t> & array) :
		pimpl(std::make_shared<impl>(array)) {
}

/*
 *
 */
IndexArray::IndexArray(const std::shared_ptr<BinaryFile> & binaryFile,
		int offset, int count) :
		pimpl(std::make_shared<impl>(binaryFile, offset, count)) {
}

/*
 *
 */
std::vector<int16_t>::const_iterator IndexArray::begin() const {
	return pimpl->buffer.cbegin();
}

/*
 *
 */
std::vector<int16_t>::const_iterator IndexArray::end() const {
	return pimpl->buffer.cend();
}

/*
 *
 */
int16_t IndexArray::get(size_t idx) const {
	return pimpl->buffer.at(idx);
}
/*
 *
 */
const int16_t * IndexArray::getBuffer() const {
	return pimpl->buffer.data();
}

/*
 *
 */
size_t IndexArray::size() const {
	return pimpl->count;
}

/*
 *
 */bool IndexArray::validate() const {
	return pimpl->validate();
}

/**
 * get script object factory for IndexArray
 *
 * @param currentDir  current directory of caller
 *
 * @return            IndexArray factory
 */
ScriptObjectPtr IndexArray::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
