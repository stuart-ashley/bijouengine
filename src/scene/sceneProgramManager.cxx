#include "sceneProgramManager.h"

#include <mutex>
#include <unordered_map>

struct SceneProgramManager::impl {
	std::mutex lock;
	std::unordered_map<std::string, std::shared_ptr<SceneProgram>> scripts;
};

/**
 * constructor
 */
SceneProgramManager::SceneProgramManager() :
		pimpl(new impl()) {
}

/*
 *
 */
void SceneProgramManager::executeLoadedCallbacks() {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	for (const auto & it : pimpl->scripts) {
		it.second->executeLoadedCallbacks();
	}
}

/*
 *
 */
const std::shared_ptr<SceneProgram> & SceneProgramManager::getScript(
		const std::string & canonicalFilename) {

	std::lock_guard<std::mutex> locker(pimpl->lock);

	auto it = pimpl->scripts.find(canonicalFilename);
	if (it == pimpl->scripts.end()) {
		auto script = std::make_shared<SceneProgram>(canonicalFilename);
		pimpl->scripts.emplace(canonicalFilename, script);
		return pimpl->scripts[canonicalFilename];
	}
	return it->second;
}

/*
 *
 */
std::vector<std::string> SceneProgramManager::getScripts() const {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	std::vector<std::string> names;
	for (const auto & pair : pimpl->scripts) {
		names.emplace_back(pair.first);
	}

	return names;
}

/*
 *
 */
void SceneProgramManager::loadScript(const std::string & canonicalFilename,
		const ScriptObjectPtr & callback) {

	std::lock_guard<std::mutex> locker(pimpl->lock);

	auto it = pimpl->scripts.find(canonicalFilename);
	if (it == pimpl->scripts.end()) {
		auto script = std::make_shared<SceneProgram>(canonicalFilename);
		pimpl->scripts.emplace(canonicalFilename, script);
		script->addLoadedCallback(callback);
	} else {
		it->second->addLoadedCallback(callback);
	}
}

/*
 *
 */
void SceneProgramManager::reload(const std::string & canonicalFilename) {
	auto sceneProgram = std::make_shared<SceneProgram>(canonicalFilename);

	std::unique_lock<std::mutex> locker(pimpl->lock);

	pimpl->scripts.emplace(canonicalFilename, sceneProgram);
}

/*
 *
 */
STATIC SceneProgramManager & SceneProgramManager::getInstance() {
	static SceneProgramManager instance;
	return instance;
}
