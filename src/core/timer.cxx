#include "timer.h"

#ifdef WIN32
#define NOMINMAX
#include "windows.h"

namespace{
	INT64 getFreq(){
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		return li.QuadPart;
	}
	INT64 getTime(){
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return li.QuadPart;
	}
}

struct Timer::impl {
	INT64 start;
	double freq;

	impl() :
		start(getTime()),
		freq(static_cast<double>(getFreq())) {
	}
};

Timer::Timer() :
pimpl(new impl()) {
}

Timer::~Timer() {
}

double Timer::get() const {
	auto dtime = getTime() - pimpl->start;
	return static_cast<double>(dtime) / pimpl->freq;
}
#else
#include <chrono>

struct Timer::impl {
	std::chrono::time_point<std::chrono::high_resolution_clock>  start;

	impl() :
			start(std::chrono::high_resolution_clock::now()) {
	}
};

Timer::Timer() :
		pimpl(new impl()) {
}

Timer::~Timer() {
}

double Timer::get() const {
	auto dtime = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now() - pimpl->start);
	return static_cast<double>(dtime.count()) / 1000000.;
}
#endif
