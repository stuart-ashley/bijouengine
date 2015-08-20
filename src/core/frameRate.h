#pragma once

#include <memory>

class FrameRate {
public:
	FrameRate();
	~FrameRate();

	/**
	 * start timing frame rate
	 */
	void begin();

	/**
	 * stop timing frame rate
	 */
	void update();

	/**
	 * get smoothed frame rate
	 *
	 * @return frame rate
	 */
	float getRate() const;
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

