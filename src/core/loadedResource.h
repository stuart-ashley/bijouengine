#pragma once

#include <string>

class LoadedResource {
public:
	LoadedResource(const std::string & canonicalPath);

	const std::string & getCanonicalPath() const {
		return m_canonicalPath;
	}

	const std::string & getMd5Sum() const {
		return m_md5sum;
	}

private:
	std::string m_canonicalPath;
	std::string m_md5sum;
};

