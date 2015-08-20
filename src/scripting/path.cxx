#include "path.h"

#include "executable.h"
#include "list.h"
#include "parameters.h"
#include "string.h"

#ifdef WIN32
#include <windows.h>

std::vector<std::string> getDirectoryContents(const std::string & directory) {
	std::vector<std::string> contents;
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;

	strcpy_s(szDir, directory.c_str());
	strcat_s(szDir, "\\*");
	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE != hFind) {
		do {
			contents.emplace_back(directory + '\\' + ffd.cFileName);
		} while (FindNextFile(hFind, &ffd) != 0);
	}

	return contents;
}

std::string getName(const std::string & path) {
	char path_buffer[_MAX_PATH];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	strcpy_s(path_buffer, path.c_str());
	_splitpath_s(path_buffer, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);
	_makepath_s(path_buffer, NULL, NULL, fname, ext);

	return path_buffer;
}

std::string getDirectory(const std::string & path) {
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];

	strcpy_s(path_buffer, path.c_str());
	_splitpath_s(path_buffer, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);
	_makepath_s(path_buffer, drive, dir, NULL, NULL);

	return path_buffer;
}

bool isDir(const std::string & path) {
	DWORD dw = GetFileAttributes(path.c_str());
	return (dw & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool isFile(const std::string & path) {
	return isDir(path) == false;
}

bool isHidden(const std::string & path) {
	DWORD dw = GetFileAttributes(path.c_str());
	return (dw & FILE_ATTRIBUTE_HIDDEN) != 0;

}

std::string getPathName(const Path & path, const std::string & name) {
	return path.getCanonicalName() + '\\' + name;
}
#else
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <cstring>

std::vector<std::string> getDirectoryContents(const std::string & directory) {
	std::vector<std::string> contents;
	DIR * dir;
	dirent * ent;

	dir = opendir(directory.c_str());
	while ((ent = readdir(dir)) != NULL) {
		contents.emplace_back(directory + "/" + ent->d_name);
	}
	closedir(dir);

	return contents;
}

std::string getName(const std::string & path) {
	char copy[path.size() + 1];
	strcpy(copy, path.c_str());
	return basename(copy);
}

std::string getDirectory(const std::string & path) {
	char copy[path.size() + 1];
	strcpy(copy, path.c_str());
	return dirname(copy);
}

bool isDir(const std::string & path) {
	struct stat attribs;
	if (stat(path.c_str(), &attribs) == 0) {
		return S_ISDIR(attribs.st_mode);
	}
	return false;
}

bool isFile(const std::string & path) {
	struct stat attribs;
	if (stat(path.c_str(), &attribs) == 0) {
		return S_ISREG(attribs.st_mode);
	}
	return false;
}

bool isHidden(const std::string & path) {
	auto name = getName(path);
	if (name.size() <= 1 || name == "..") {
		return false;
	}
	return name[0] == '.';
}

std::string getPathName(const Path & path, const std::string & name) {
	return path.getCanonicalName() + "/" + name;
}
#endif

namespace {

	class GetCanonicalName: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto file = std::static_pointer_cast<Path>(self);
			stack.emplace(std::make_shared<String>(file->getCanonicalName()));
		}
	};

	class GetDirectoryContents: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto file = std::static_pointer_cast<Path>(self);
			std::vector<ScriptObjectPtr> list;
			for (const auto & item : file->getDirectoryContents()) {
				list.emplace_back(item);
			}
			stack.emplace(std::make_shared<List>(list));
		}
	};

	class GetName: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto file = std::static_pointer_cast<Path>(self);
			stack.emplace(std::make_shared<String>(file->getName()));
		}
	};

	class GetParentDirectory: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			const auto file = std::static_pointer_cast<Path>(self);
			stack.emplace(std::make_shared<Path>(file->getParent()));
		}
	};

	class IsDir: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			if (std::static_pointer_cast<Path>(self)->isDir()) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class IsFile: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			if (std::static_pointer_cast<Path>(self)->isFile()) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};

	class IsHidden: public Executable {
		void execute(const ScriptObjectPtr & self, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			checkNumArgs(nArgs, 0);

			if (std::static_pointer_cast<Path>(self)->isHidden()) {
				stack.emplace(Bool::True());
			} else {
				stack.emplace(Bool::False());
			}
		}
	};
}

/**
 *
 */
Path::Path(const std::string & canonicalPath) :
		m_canonicalName(canonicalPath), m_name(::getName(canonicalPath)) {
}

/**
 *
 */
Path::~Path() {
}

/**
 *
 */
const std::string & Path::getCanonicalName() const {
	return m_canonicalName;
}

/**
 *
 */
std::vector<std::shared_ptr<Path>> Path::getDirectoryContents() const {
	auto listing = ::getDirectoryContents(m_canonicalName);
	std::vector<std::shared_ptr<Path>> contents;
	for (const auto & str : listing) {
		contents.emplace_back(std::make_shared<Path>(str));
	}
	return contents;
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
OVERRIDE ScriptObjectPtr Path::getMember(ScriptExecutionState & execState,
		const std::string & name) const {
	static std::unordered_map<std::string, ScriptObjectPtr> members = {
			{ "getCanonicalName", std::make_shared<GetCanonicalName>() },
			{ "getDirectoryContents", std::make_shared<GetDirectoryContents>() },
			{ "getName", std::make_shared<GetName>() },
			{ "getParent", std::make_shared<GetParentDirectory>() },
			{ "isDir", std::make_shared<IsDir>() },
			{ "isFile", std::make_shared<IsFile>() },
			{ "isHidden", std::make_shared<IsHidden>() } };

	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return ScriptObject::getMember(execState, name);
}

/**
 *
 */
const std::string & Path::getName() const {
	return m_name;
}

/**
 *
 */
Path Path::getParent() const {
	return Path(getDirectory(m_canonicalName));
}

/**
 *
 */
bool Path::isDir() const {
	return ::isDir(m_canonicalName);
}

/**
 *
 */
bool Path::isFile() const {
	return ::isFile(m_canonicalName);
}

/**
 *
 */
bool Path::isHidden() const {
	return ::isHidden(m_canonicalName);
}

/**
 *
 */
OVERRIDE std::string Path::toString() const {
	return m_canonicalName;
}
