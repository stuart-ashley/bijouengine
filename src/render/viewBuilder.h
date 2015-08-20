#pragma once

#include <memory>
#include <vector>

class BoundingBox;
class ConvexHull;
class DebugGeometry;
class IndexArray;
class Plane;

namespace render {
	class IndexedTriangles;
	class Lighting;
	class RenderState;
	class Uniform;
	class View;

	class ViewBuilder {
	public:

		/**
		 * Constructor
		 */
		ViewBuilder(const std::shared_ptr<View> & view,
				const Lighting & lighting);

		/**
		 * destructor
		 */
		~ViewBuilder();

		void addAlphaIndexedTriangles(const IndexedTriangles & indices,
				float z);

		void addDebugGeometry(const std::vector<DebugGeometry> & debug);

		void addEdgeIndexArray(const IndexArray & indices);

		void addIndexedTriangles(const IndexedTriangles & indices);

		/**
		 * Add occluder to view
		 */
		void addOccluders(const std::vector<ConvexHull> & occluders);

		void addPoint();

		void addSphere(float radius);

		/**
		 * add extra view required by current view
		 *
		 * @param view  extra view
		 */
		void addView(const std::shared_ptr<View> & view);

		/**
		 * add plane for volumetric lighting
		 *
		 * @param triangles  triangle indices for region to apply volumetric effect to
		 */
		void addVolumetric(const IndexedTriangles & triangles);

		/**
		 * Apply reflection texture uniform for plane, firstly try find existing, if
		 * that fails create new
		 *
		 * @param worldPlane
		 *            plane of reflection
		 */
		void applyMirror(const std::string & name, const Plane & worldPlane);

		/**
		 * Apply refraction texture uniform for plane, firstly try find existing, if
		 * that fails create new
		 *
		 * @param worldPlane
		 *            plane of refraction
		 */
		void applyRefraction(const std::string & name,
				const Plane & worldPlane);

		/**
		 * get additional view tasks for view
		 *
		 * @return  views required for view
		 */
		const std::vector<std::shared_ptr<View>> & getAdditionalViewTasks() const;

		size_t getPolyCount() const;

		/**
		 * get type of shadows
		 *
		 * @return  type of shadows
		 */
		size_t getShadowType() const;

		RenderState & getState() const;

		const std::shared_ptr<View> & getView() const;

		void incPolyCount(size_t n);

		/**
		 * Check bounding box is visible, by first checking for intersection with
		 * camera frustum, and then testing against occlusion geometry
		 *
		 * @param box
		 * @return
		 */
		bool isVisible(const BoundingBox & box) const;

		/**
		 * Pop state
		 */
		void popState();

		/**
		 * Push state
		 */
		void pushState();

		/**
		 * Show collisions
		 */
		bool showCollisions();

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

