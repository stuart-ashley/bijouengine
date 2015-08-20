#include "loadedResource.h"
#include "loadingCallback.h"
#include "loadManagerRecursive.h"
#include "loadManagerUtils.h"

#include <cassert>
#include <fstream>
#include <iostream>

LoadManagerRecursive::~LoadManagerRecursive() {
}

void LoadManagerRecursive::start(const std::string & dir) {
	setRootPath(dir);
}

void LoadManagerRecursive::stop() {
}

void LoadManagerRecursive::load(LoadingCallback & cb) {
	std::ifstream in(cb.getCanonicalFilename(), std::ios::binary);

	// loading callback
	try {
		cb.load(in);
	} catch (std::exception & e) {
		std::cerr << "ERROR: Failed to load '" << cb.getCanonicalFilename()
				<< "'" << std::endl;
		assert(false);
	}

	// update loaded
	addLoaded(LoadedResource(cb.getCanonicalFilename()));
}

void LoadManagerRecursive::flush() {
}
