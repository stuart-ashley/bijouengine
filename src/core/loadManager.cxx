#include "loadedResource.h"
#include "loadingCallback.h"
#include "loadManager.h"
#include "loadManagerUtils.h"

#include <cassert>
#include <iostream>
#include <mutex>
#include <unordered_map>

namespace {
	static std::shared_ptr<LoadManager> instance = nullptr;
}

struct LoadManager::impl {
	std::mutex m_lock;
	std::unordered_map<std::string, LoadedResource> loaded;
	std::string rootPath;
};

LoadManager::LoadManager() :
		pimpl(new impl()) {
}

LoadManager::~LoadManager() {
}

std::string LoadManager::getName(const std::string & currentPath,
		const std::string & name) const {
	return LoadManagerUtils::getCanonicalName(currentPath, name,
			pimpl->rootPath);
}

const std::shared_ptr<LoadManager> & LoadManager::getInstance() {
	return instance;
}

void LoadManager::setInstance(
		const std::shared_ptr<LoadManager> & loadManager) {
	instance = loadManager;
}

void LoadManager::addLoaded(const LoadedResource & resource) {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);

	// attempt insert
	auto result = pimpl->loaded.emplace(resource.getMd5Sum(), resource);
	if (result.second == false) {
		if (result.first->second.getCanonicalPath()
				!= resource.getCanonicalPath()) {
			std::cerr << "Hash collision: "
					<< result.first->second.getCanonicalPath()
					<< " same hash as " << resource.getCanonicalPath();
			assert(false);
		} else {
			std::cerr << "Reloading: " << resource.getCanonicalPath();
		}
	}
}

bool LoadManager::isLoaded(const std::string & canonicalFilename) {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);

	for (const auto & entry : pimpl->loaded) {
		if (canonicalFilename == entry.second.getCanonicalPath()) {
			return true;
		}
	}
	return false;
}

void LoadManager::setRootPath(const std::string & rootPath) {
#ifdef WIN32
	if (rootPath.back() == '\\') {
		pimpl->rootPath = LoadManagerUtils::getCanonicalName("", rootPath.substr(0, rootPath.size() - 1), "") + '\\';
	} else {
		pimpl->rootPath = LoadManagerUtils::getCanonicalName("", rootPath, "") + '\\';
	}
#else
	if (rootPath.back() == '/') {
		pimpl->rootPath = LoadManagerUtils::getCanonicalName("", rootPath.substr(0, rootPath.size() - 1), "") + '/';
	} else {
		pimpl->rootPath = LoadManagerUtils::getCanonicalName("", rootPath, "") + '/';
	}
#endif
}
