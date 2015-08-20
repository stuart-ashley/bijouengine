#pragma once

#include <memory>
#include <string>
#include <thread>

class Draw;

class Update {
public:
	Update(const std::string & dataDir, Draw & draw);
	~Update();

	void run();
	void stop();
	std::thread spawn();
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

