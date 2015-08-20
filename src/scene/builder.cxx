#include "builder.h"

#include "lights.h"
#include "physics.h"
#include "taskInitNode.h"
#include "taskWrapper.h"

#include "../core/aspect.h"
#include "../core/color.h"
#include "../core/config.h"
#include "../core/debugGeometry.h"
#include "../core/transform.h"

#include "../render/clearScreen.h"
#include "../render/renderGraph.h"
#include "../render/shadow.h"
#include "../render/spotlight.h"
#include "../render/sunlight.h"

#include <cassert>
#include <deque>
#include <iostream>
#include <mutex>
#include <stack>
#include <thread>
#include <unordered_map>

using namespace render;

struct Builder::impl {
	std::mutex m_lock;

	std::shared_ptr<RenderGraph> renderGraph;
	std::vector<ConvexHull> occluders;
	Lights lights;
	std::stack<Transform> transformStack;
	int id;
	size_t polyCount;
	std::vector<DebugGeometry> debugGeometry;

	std::deque<std::thread> threads;

	static int uid;

	impl(const std::vector<DebugGeometry> & debug) :
					renderGraph(std::make_shared<RenderGraph>()),
					id(++uid),
					polyCount(0),
					debugGeometry(debug) {

	}

	void addTask(std::shared_ptr<TaskWrapper> taskWrapper) {
		taskWrapper->init();
		renderGraph->addTask(taskWrapper->getRenderTask());
	}
};

int Builder::impl::uid = 0;

/**
 * Constructor
 *
 * @param scene
 * @param renderGraph
 */
Builder::Builder(const std::vector<DebugGeometry> & debug,
		const std::vector<std::shared_ptr<TaskInitNode>> & tasks) :
		pimpl(new impl(debug)) {
	pimpl->transformStack.push(Transform());
	pimpl->id = ++pimpl->uid;

	pimpl->renderGraph->addTask(std::make_shared<ClearScreen>(""));

	// initialise windows and grab the final position of cameras
	for (const auto & node : tasks) {
		node->taskInit(*this);
	}

	if (Config::getInstance().getBoolean("showProjectedKaleidoscopes")) {
		const auto & kaleidoscopeDebug =
				pimpl->lights.getProjectedKaleidoscopesDebug();
		pimpl->debugGeometry.insert(pimpl->debugGeometry.end(),
				kaleidoscopeDebug.begin(), kaleidoscopeDebug.end());
	}
	if (Config::getInstance().getBoolean("showProjectedTextures")) {
		const auto & textureDebug = pimpl->lights.getProjectedTexturesDebug();
		pimpl->debugGeometry.insert(pimpl->debugGeometry.end(),
				textureDebug.begin(), textureDebug.end());
	}
}

/**
 * destructor
 */
Builder::~Builder() {
}

/**
 * Add task
 */
void Builder::addTask(const std::shared_ptr<TaskWrapper> & taskWrapper) {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);
	pimpl->threads.emplace_back(&impl::addTask, pimpl.get(), taskWrapper);
}

/**
 * Execute builder
 */
std::shared_ptr<render::RenderGraph> & Builder::execute() {
	std::unique_lock<std::mutex> locker(pimpl->m_lock);

	while (pimpl->threads.empty() == false) {
		auto thread = std::move(pimpl->threads.front());
		pimpl->threads.pop_front();
		locker.unlock();

		thread.join();

		locker.lock();
	}
	locker.unlock();

	return pimpl->renderGraph;
}

/**
 * get debug geometry
 *
 * @return  debug geometry
 */
const std::vector<DebugGeometry> & Builder::getDebugGeometry() const {
	return pimpl->debugGeometry;
}

/**
 * get builder id
 *
 * @return  builder id
 */
int Builder::getId() const {
	return pimpl->id;
}

/**
 * get lights
 *
 * @return  lights
 */
Lights & Builder::getLights() const {
	return pimpl->lights;
}

/**
 * get polygon count
 *
 * @return  polygon count
 */
size_t Builder::getPolyCount() const {
	return pimpl->polyCount;
}

/**
 * get occluders
 *
 * @return  occluders
 */
const std::vector<ConvexHull> & Builder::getOccluders() const {
	return pimpl->occluders;
}

/**
 * get current transform
 *
 * @return  current transform
 */
const Transform & Builder::getTransform() const {
	return pimpl->transformStack.top();
}

/**
 * increment pass polygon count
 *
 * @param n  number of extra polygons
 */
void Builder::incPolyCount(size_t n) {
	std::lock_guard<std::mutex> locker(pimpl->m_lock);
	pimpl->polyCount += n;
}

/**
 * Pop state
 */
void Builder::popState() {
	pimpl->transformStack.pop();
}

/**
 * Push state
 */
void Builder::pushState() {
	pimpl->transformStack.push(pimpl->transformStack.top());
}

void Builder::rotate(const Quat & rotation) {
	pimpl->transformStack.top().rotate(rotation);
}

void Builder::transform(const Transform & t) {
	pimpl->transformStack.top().transform(t);
}

void Builder::translate(const Vec3 & translation) {
	pimpl->transformStack.top().translate(translation);
}
