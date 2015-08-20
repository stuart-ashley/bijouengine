#pragma once

#include "loadingCallback.h"

#include <memory>
#include <string>

class BinaryFile: public LoadingCallback {
public:

	BinaryFile(const std::string & currentDir, const std::string & name);

	~BinaryFile();

	const char * getData() const;

	size_t getSize() const;

	bool valid() const;

private:
	void load(std::istream & stream);

	char * m_data;
	size_t m_dataSize;
	bool m_valid;
};

