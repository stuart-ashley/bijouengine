#pragma once

#include "scripting/scriptObject.h"

#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace render{
	class RenderGraph;
}

class Draw {
public:
#ifdef WIN32
	Draw(HINSTANCE hInstance);
#else
	Draw();
#endif

	~Draw();

	std::unordered_map<std::string, std::vector<ScriptObjectPtr>> getEvents() const;

	unsigned getHeight() const;

	float getRenderRate() const;

	unsigned getWidth() const;

	void run();

	void setRenderGraph(const std::shared_ptr<render::RenderGraph> & rg);

	std::thread spawn();

	void stop();

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

