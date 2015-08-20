#pragma once

#include "../core/transform.h"

#include <memory>
#include <vector>

class ConvexHull;
class DebugGeometry;
class Lights;
class Physics;
class SgNode;
class TaskInitNode;
class TaskWrapper;

namespace render {
	class RenderGraph;
	class RenderTask;
	class View;
}

class Builder {
public:

	/**
	 * Constructor
	 *
	 * @param scene
	 * @param renderGraph
	 */
	Builder(const std::vector<DebugGeometry> & debug,
			const std::vector<std::shared_ptr<TaskInitNode>> & tasks);

	/**
	 * destructor
	 */
	~Builder();

	/**
	 * Add task
	 */
	void addTask(const std::shared_ptr<TaskWrapper> & taskWrapper);

	/**
	 * Execute builder
	 */
	std::shared_ptr<render::RenderGraph> & execute();

	/**
	 * get debug geometry
	 *
	 * @return  debug geometry
	 */
	const std::vector<DebugGeometry> & getDebugGeometry() const;

	/**
	 * get builder id
	 *
	 * @return  builder id
	 */
	int getId() const;

	/**
	 * get lights
	 *
	 * @return  lights
	 */
	Lights & getLights() const;

	/**
	 * get occluders
	 *
	 * @return  occluders
	 */
	const std::vector<ConvexHull> & getOccluders() const;

	/**
	 * get polygon count
	 *
	 * @return  polygon count
	 */
	size_t getPolyCount() const;

	/**
	 * get current transform
	 *
	 * @return  current transform
	 */
	const Transform & getTransform() const;

	/**
	 * increment pass polygon count
	 *
	 * @param n  number of extra polygons
	 */
	void incPolyCount(size_t n);

	/**
	 * Pop state
	 */
	void popState();

	/**
	 * Push state
	 */
	void pushState();

	void rotate(const Quat & rotation);

	void transform(const Transform & t);

	void translate(const Vec3 & translation);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

