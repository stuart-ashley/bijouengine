#include "loadedResource.h"
#include "loadManagerUtils.h"

LoadedResource::LoadedResource(const std::string & canonicalPath) :
				m_canonicalPath(canonicalPath),
				m_md5sum(LoadManagerUtils::md5sum(canonicalPath)) {
}
