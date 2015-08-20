#include "binaryFileCache.h"

#include "binaryFile.h"
#include "loadManager.h"

#include <unordered_map>

STATIC const std::shared_ptr<BinaryFile> & BinaryFileCache::get(
		const std::string & currentDir, const std::string & filename) {
	static std::unordered_map<std::string, std::shared_ptr<BinaryFile>> cache;

	auto absoluteFilename = LoadManager::getInstance()->getName(currentDir,
			filename);

	auto it = cache.find(absoluteFilename);
	if (it != cache.end()) {
		return it->second;
	}

	cache.emplace(absoluteFilename,
			std::make_shared<BinaryFile>(currentDir, filename));

	return cache.find(absoluteFilename)->second;
}
