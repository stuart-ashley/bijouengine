#pragma once

#include "binaryFile.h"

#include "../scripting/scriptObject.h"

#include <memory>

class Binary final: public ScriptObject {
public:

	/**
	 * constructor
	 *
	 * @param name
	 * @param binaryFile
	 * @param offset
	 * @param count
	 */
	Binary(const std::string & name,
			const std::shared_ptr<BinaryFile> & binaryFile, int offset,
			int count);

	/**
	 * default destructor
	 */
	inline virtual ~Binary() = default;

	inline int getByteCount() const {
		return count;
	}

	inline const char * getData() const {
		return binaryFile->getData() + offset;
	}

	inline const std::string & getName() const {
		return name;
	}

	inline bool valid() const {
		return binaryFile->valid();
	}

	/**
	 * get script object factory for Binary
	 *
	 * @param currentDir  current directory of caller
	 *
	 * @return            Binary factory
	 */
	static ScriptObjectPtr getFactory(const std::string & currentDir);

private:
	std::string name;
	int offset;
	int count;
	std::shared_ptr<BinaryFile> binaryFile;
};

