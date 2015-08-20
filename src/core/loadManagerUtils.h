#pragma once

#include <string>

class LoadManagerUtils {
public:
	static std::string getCanonicalName(const std::string & currentPath,
			const std::string & name, const std::string & rootPath);

	static std::string getDirectory(const std::string & filename);

	static std::string md5sum(const std::string & filename);
};

