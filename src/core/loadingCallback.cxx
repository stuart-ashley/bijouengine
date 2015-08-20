#include "loadingCallback.h"

#include "loadManager.h"

/*
 *
 */
LoadingCallback::LoadingCallback(const std::string & currentDir,
		const std::string & filename) :
				m_canonicalFilename(
						LoadManager::getInstance()->getName(currentDir,
								filename)) {
}

/*
 *
 */
LoadingCallback::~LoadingCallback() {
}
