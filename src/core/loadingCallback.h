#pragma once

#include <istream>

class LoadingCallback {
public:
	LoadingCallback(const std::string & currentDir,
			const std::string & filename);

	virtual ~LoadingCallback();

	virtual void load(std::istream & stream) = 0;

	inline const std::string & getCanonicalFilename() {
		return m_canonicalFilename;
	}
private:
	std::string m_canonicalFilename;
};

