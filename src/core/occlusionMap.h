#pragma once

#include <memory>

class Aspect;
class ConvexHull;
class Transform;

class OcclusionMap {
public:

	/**
	 * Create new occlusion map
	 *
	 * @param aspect
	 * @param width
	 * @param height
	 */
	OcclusionMap(const Aspect & aspect, int width, int height);

	/**
	 * destructor
	 */
	~OcclusionMap();

	/**
	 * Test occluder against occlusion map
	 *
	 * @param aspect
	 * @param toWorld
	 * @param occluder
	 * @return
	 */
	bool isOccluded(const Transform & toWorld,
			const ConvexHull & occluder) const;

	/**
	 * Write occluder to occlusion map
	 *
	 * @param aspect
	 * @param occluder
	 * @param debugGeometry
	 */
	void write(const ConvexHull & occluder);

private:
	struct impl;
	std::unique_ptr<impl> pimpl;
};

