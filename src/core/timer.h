#pragma once

#include <memory>

class Timer {
public:
	Timer();
	~Timer();

	double get() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};


