#pragma once

#include "../scripting/scriptObject.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

class BinaryFile;

class IndexArray final: public ScriptObject {
public:
	IndexArray(const std::initializer_list<int16_t> & list);

	IndexArray(const std::vector<int16_t> & array);

	/**
	 * Set indices from file
	 *
	 * @param binaryFile  binary file containing indices
	 * @param offset      byte offset to indices in binary file
	 * @param count       index count
	 */
	IndexArray(const std::shared_ptr<BinaryFile> & binaryFile, int offset,
			int count);

	/**
	 * default destructor
	 */
	inline virtual ~IndexArray() = default;

	std::vector<int16_t>::const_iterator begin() const;

	std::vector<int16_t>::const_iterator end() const;

	int16_t get(size_t idx) const;

	const int16_t * getBuffer() const;

	inline bool operator==(const IndexArray & other) const {
		return pimpl == other.pimpl;
	}

	size_t size() const;

	bool validate() const;

	/**
	 * get script object factory for IndexArray
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            IndexArray factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);
private:
	struct impl;
	std::shared_ptr<impl> pimpl;
};

