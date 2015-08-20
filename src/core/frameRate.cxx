#include "frameRate.h"

#include <algorithm>
#include <chrono>
#include <mutex>

namespace {
	const int maxBufferSize = 10;
}

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
/*
 *
 */
struct FrameRate::impl {
	INT64 freq;
	INT64 time;
	int bufferSize;
	int position;
	INT64 deltas[maxBufferSize];

	std::mutex lock;

	impl() :
			freq(getFreq()), bufferSize(0), position(0), deltas{} {
	}
};

/*
 *
 */
FrameRate::FrameRate() :
		pimpl(new impl()) {
}

/*
 *
 */
FrameRate::~FrameRate() {
}

/*
 *
 */
void FrameRate::begin() {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	pimpl->time = getTime();
}

/*
 *
 */
void FrameRate::update() {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	pimpl->deltas[pimpl->position] = getTime() - pimpl->time;

	++pimpl->position;

	pimpl->bufferSize = std::max(pimpl->bufferSize, pimpl->position);

	if (pimpl->position >= maxBufferSize) {
		pimpl->position = 0;
	}

	pimpl->time = getTime();
}

/*
 *
 */
float FrameRate::getRate() const {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	INT64 adt = 0;
	for (const auto & delta : pimpl->deltas) {
		adt += delta;
	}
	return static_cast<float>(pimpl->freq) *
			static_cast<float>(pimpl->bufferSize) / static_cast<float>(adt);
}
#else
/*
 *
 */
struct FrameRate::impl {
	std::chrono::high_resolution_clock::time_point time;
	int bufferSize;
	int position;
	std::chrono::microseconds deltas[maxBufferSize];

	std::mutex lock;

	impl() :
		bufferSize(0), position(0), deltas{} {
	}
};

/*
 *
 */
FrameRate::FrameRate() :
		pimpl(new impl()) {
}

/*
 *
 */
FrameRate::~FrameRate() {
}

/*
 *
 */
void FrameRate::begin() {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	pimpl->time = std::chrono::high_resolution_clock::now();
}

/*
 *
 */
void FrameRate::update() {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	pimpl->deltas[pimpl->position] = std::chrono::duration_cast<
			std::chrono::microseconds>(
			std::chrono::high_resolution_clock::now() - pimpl->time);

	++pimpl->position;

	pimpl->bufferSize = std::max(pimpl->bufferSize, pimpl->position);

	if (pimpl->position >= maxBufferSize) {
		pimpl->position = 0;
	}

	pimpl->time = std::chrono::high_resolution_clock::now();
}

/*
 *
 */
float FrameRate::getRate() const {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	std::chrono::microseconds adt = std::chrono::microseconds::zero();
	for (const auto & delta : pimpl->deltas) {
		adt += delta;
	}
	return 1000000.f * static_cast<float>(pimpl->bufferSize)
			/ static_cast<float>(adt.count());
}
#endif