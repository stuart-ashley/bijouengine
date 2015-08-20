#pragma once

#include <memory>
#include <string>

class BinaryFile;

class BinaryFileCache {
public:
	static const std::shared_ptr<BinaryFile> & get(
			const std::string & currentDir, const std::string & filename);
};

