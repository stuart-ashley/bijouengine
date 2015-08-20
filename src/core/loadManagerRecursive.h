#pragma once

#include "loadManager.h"

#include <string>

class LoadManagerRecursive: public LoadManager {
public:
	~LoadManagerRecursive();

	void start(const std::string & dir) override;

	void stop() override;

	void load(LoadingCallback & cb) override;

	void flush() override;
};

