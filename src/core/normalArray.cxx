#include "normalArray.h"

#include "binaryFile.h"
#include "binaryFileCache.h"
#include "mat3.h"
#include "normal.h"

#include "../scripting/executable.h"
#include "../scripting/list.h"
#include "../scripting/parameter.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/scriptExecutionException.h"
#include "../scripting/string.h"

#include <atomic>

namespace {

	/*
	 *
	 */
	std::vector<BaseParameter> params = { Parameter<String>("file", nullptr),
			Parameter<Real>("offset", nullptr), Parameter<Real>("count",
					nullptr) };

	/*
	 *
	 */
	struct Factory: public Executable {
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

				std::vector<Normal> normals;
				normals.reserve(list.size());

				for (const auto & e : list) {
					scriptExecutionAssertType<Normal>(e,
							"Require list of normals");

					normals.emplace_back(*std::static_pointer_cast<Normal>(e));
				}

				stack.emplace(std::make_shared<NormalArray>(normals));

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
					std::make_shared<NormalArray>(binFile, offset->getInt32(),
							count->getInt32()));
		}
	};
}

struct NormalArray::impl {
	std::vector<Normal> m_normals;
	std::shared_ptr<BinaryFile> m_binaryFile;
	int m_offset;
	int m_count;
	std::atomic<bool> m_valid;

	impl(const std::vector<Normal> & normals) :
			m_normals(normals), m_offset(0), m_count(0), m_valid(true) {
	}

	impl(const std::shared_ptr<BinaryFile> & binaryFile, int offset, int count) :
					m_binaryFile(binaryFile),
					m_offset(offset),
					m_count(count),
					m_valid(false) {
	}

	bool validate() {
		if (m_valid) {
			return true;
		}
		if (m_binaryFile->valid()) {
			m_normals.reserve(m_count);

			const float * floats =
					reinterpret_cast<const float *>(m_binaryFile->getData()
							+ m_offset);

			for (int i = 0; i < m_count; ++i) {
				auto x = *floats++;
				auto y = *floats++;
				auto z = *floats++;
				m_normals.emplace_back(x, y, z);
			}

			m_valid = true;
			return true;
		}
		return false;
	}
};

/**
 * construct from initializer list
 */
NormalArray::NormalArray(const std::initializer_list<Normal> & list) :
		pimpl(new impl(list)) {
}

/**
 * Construct from Normal list
 */
NormalArray::NormalArray(const std::vector<Normal> & normals) :
		pimpl(new impl(normals)) {
}

/**
 * Construct from BinaryFile
 */
NormalArray::NormalArray(const std::shared_ptr<BinaryFile> & binaryFile,
		int offset, int count) :
		pimpl(new impl(binaryFile, offset, count)) {
}

/**
 * destructor
 */
NormalArray::~NormalArray() {
}

/**
 * add normal to array
 *
 * @param n  normal to add
 */
void NormalArray::add(const Normal & n) {
	pimpl->m_normals.emplace_back(n);
}

/*
 *
 */
std::vector<Normal>::const_iterator NormalArray::begin() const {
	return pimpl->m_normals.cbegin();
}

/*
 *
 */
std::vector<Normal>::const_iterator NormalArray::end() const {
	return pimpl->m_normals.cend();
}

/**
 * Get normal at specified index
 *
 * @param index  index to fetch
 *
 * @return       normal at index
 */
const Normal & NormalArray::get(size_t index) const {
	return pimpl->m_normals.at(index);
}

/**
 * get number of normals in array
 *
 * @return  size of array
 */
size_t NormalArray::size() const {
	return pimpl->m_normals.size();
}

/**
 * Produce a rotated version of this array
 *
 * @param r  rotation matrix
 *
 * @return   rotated normal array
 */
NormalArray NormalArray::transformed(const Mat3 & r) const {
	std::vector<Normal> normals;
	normals.reserve(pimpl->m_normals.size());

	for (const auto & normal : pimpl->m_normals) {
		normals.emplace_back(r * normal);
	}

	return NormalArray(normals);
}

/**
 * If the array is from a file make sure it's loaded
 *
 * @return  true if loaded, false if not
 */
bool NormalArray::validate() const {
	return pimpl->validate();
}

/**
 * get script object factory for NormalArray
 *
 * @param currentDir  current directory of caller
 *
 * @return            NormalArray factory
 */
STATIC ScriptObjectPtr NormalArray::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}
