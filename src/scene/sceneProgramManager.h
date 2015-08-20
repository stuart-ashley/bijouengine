#pragma once

#include "sceneProgram.h"

class SceneProgramManager {
public:

	/**
	 * constructor
	 */
	SceneProgramManager();

	void executeLoadedCallbacks();

	const std::shared_ptr<SceneProgram> & getScript(
			const std::string & canonicalFilename);

	std::vector<std::string> getScripts() const;

	void loadScript(const std::string & canonicalFilename,
			const ScriptObjectPtr & callback);

	void reload(const std::string & canonicalFilename);

	static SceneProgramManager & getInstance();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

