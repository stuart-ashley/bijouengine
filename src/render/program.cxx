#include "program.h"
#include "../scripting/bool.h"
#include "../core/loadingCallback.h"
#include "../core/loadManager.h"
#include "../render/shaderFlag.h"

#include <vector>

using namespace render;

struct Program::impl: public LoadingCallback {
	std::string m_source;
	ShaderFlags m_validFlags;
	bool m_loaded;

	impl(const std::string & currentDir, const std::string & filename) :
					LoadingCallback(currentDir, filename),
					m_loaded(false) {
		LoadManager::getInstance()->load(*this);
	}

	void load(std::istream & stream) {
		stream.seekg(0, std::ios::end);
		m_source.reserve(stream.tellg());
		stream.seekg(0, std::ios::beg);

		m_source.assign((std::istreambuf_iterator<char>(stream)),
				std::istreambuf_iterator<char>());

		int idx = 0;
		for (const auto & flag : ShaderFlag::values()) {
			if (m_source.find(flag)) {
				m_validFlags[idx] = true;
			}
			++idx;
		}
		m_loaded = true;
	}
};

/*
 *
 */
Program::Program(const std::string & currentDir, const std::string & filename) :
		pimpl(new impl(currentDir, filename)) {
}

/*
 *
 */
Program::~Program() {
}

/*
 *
 */
const std::string & Program::getName() const {
	return pimpl->getCanonicalFilename();
}

/*
 *
 */
const std::string & Program::getSource() const {
	return pimpl->m_source;
}

/*
 *
 */
const ShaderFlags & Program::getValidFlags() const {
	return pimpl->m_validFlags;
}

/*
 *
 */
bool Program::isLoaded() const {
	return pimpl->m_loaded;
}
