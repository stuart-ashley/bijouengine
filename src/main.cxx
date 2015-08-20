#include "draw.h"
#include "update.h"

#include "core/config.h"
#include "core/loadManager.h"
#include "core/loadManagerRecursive.h"
#include "core/loadManagerUtils.h"

#include "scripting/bool.h"
#include "scripting/procedure.h"
#include "scripting/program.h"
#include "scripting/real.h"
#include "scripting/scriptExecutionState.h"
#include "scripting/string.h"

#include <cassert>
#include <fstream>
#include <memory>
#include <stack>
#include <thread>

#ifdef WIN32
#include <Shlobj.h>

std::string getHomeDirectory() {
	char path[_MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, path))) {
		return path;
	}
	return "";
}
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

std::string getHomeDirectory() {
	const char *homedir = getenv("HOME");

	if (homedir == NULL) {
		homedir = getpwuid(getuid())->pw_dir;
		if (homedir == NULL) {
			return "";
		}
	}

	return homedir;
}
#endif

namespace{

	void initConfig(const std::string & dataDir, const std::string & configFile) {
		// config defaults
		Config::getInstance().set("generators", Bool::True());
		Config::getInstance().set("simulationSpeed", std::make_shared<Real>(1));
		Config::getInstance().set("sunShadow",
			std::make_shared<String>("CASCADE_PCF_SHADOWS"));
		Config::getInstance().set("width", std::make_shared<Real>(0));
		Config::getInstance().set("height", std::make_shared<Real>(0));
		Config::getInstance().set("debugPort", std::make_shared<Real>(-1));
		Config::getInstance().set("home", std::make_shared<String>(getHomeDirectory()));

		// load config
		Config::getInstance().init(dataDir, configFile);
	}
}

#ifdef WIN32
#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	std::string dataDir;

	TCHAR buffer[_MAX_PATH];
	DWORD dwRet = GetModuleFileName(NULL, buffer, _MAX_PATH);
	if (dwRet != 0) {
		dataDir = LoadManagerUtils::getDirectory(buffer);
	}

	std::string configFilename = "config.script";

	LoadManager::setInstance(std::make_shared<LoadManagerRecursive>());
	LoadManager::getInstance()->start(dataDir);

	initConfig(dataDir, configFilename);

	Draw draw(hInstance);

	Update update(dataDir, draw);

	auto drawThread = draw.spawn();

	auto updateThread = update.spawn();

	drawThread.join();

	update.stop();
	updateThread.join();
}
#else
#include <unistd.h>
#include <libgen.h>

int main(int argc, char * argv[]) {
	char path[256];
	char id[256];
	sprintf(id, "/proc/%d/exe", getpid());
	readlink(id, path, 255);
	path[255] = '\0';

	std::string dataDir = std::string(dirname(path)) + "/../data/";

	std::string configFilename = "config.script";
	if (argc > 1) {
		configFilename = argv[1];
	}

	LoadManager::setInstance(std::make_shared<LoadManagerRecursive>());
	LoadManager::getInstance()->start(dataDir);

	initConfig(dataDir, configFilename);

	Draw draw;

	Update update(dataDir, draw);

	auto drawThread = draw.spawn();

	auto updateThread = update.spawn();

	drawThread.join();

	update.stop();
	updateThread.join();
}
#endif
