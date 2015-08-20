#pragma once

#include "renderTask.h"

#include <bitset>

class Aspect;
class BoundingBox;
class DebugGeometry;
class IndexArray;
class Transform;

namespace render {
	class IndexedTriangles;
	class RenderState;
	class ShaderTag;
	class UniformArray;

	class View: public RenderTask {
	public:
		enum class Cull {
			NONE, BACK, FRONT, FRONTBACK
		};

		enum class Type {
			SHADOW, REGULAR
		};

		enum Modifier {
			LORES, WIREFRAME, SNAPSHOT, maxValue
		};
		typedef std::bitset<Modifier::maxValue> ModifierSet;

		enum class DepthCompare {
			LESS, GREATER
		};

		View(const std::string & name, const Aspect & aspect,
				const Aspect & lodAspect,
				const std::vector<std::shared_ptr<Texture>> & textures,
				const Rect & rect, const UniformArray & uniforms,
				const ShaderTag & shader, Cull cull, DepthCompare depthCompare,
				const ModifierSet & modifiers, int level);

		View(const std::string & name, const Aspect & aspect,
				const Aspect & lodAspect,
				const std::shared_ptr<Texture> & texture, const Rect & rect,
				const UniformArray & uniforms, const ShaderTag & shader,
				Cull cull, DepthCompare depthCompare,
				const ModifierSet & modifiers, int level);

		/**
		 * destructor
		 */
		~View();

		virtual void addAlphaIndexedTriangles(const RenderState & state,
				const IndexedTriangles & indices, float z) = 0;

		virtual void addDebugGeometry(
				const std::vector<DebugGeometry> & debug) = 0;

		virtual void addEdgeIndexArray(const RenderState & state,
				const IndexArray & indices) = 0;

		virtual void addIndexedTriangles(const RenderState & state,
				const IndexedTriangles & indices) = 0;

		virtual void addPoint(const RenderState & state) = 0;

		virtual void addSphere(const RenderState & state, float radius) = 0;

		virtual void addVolumetric(const RenderState & state,
				const IndexedTriangles & indices) = 0;

		virtual void execute(Canvas & canvas) override = 0;

		/**
		 * get aspect for view
		 *
		 * @return  view aspect
		 */
		const Aspect & getAspect() const;

		/**
		 * get culling mode for view
		 *
		 * @return  view culling mode
		 */
		Cull getCull() const;

		/**
		 * get depth compare mode for view
		 *
		 * @return  view depth compare mode
		 */
		DepthCompare getDepthCompare() const;

		/**
		 * get lod aspect for view
		 *
		 * @return  view lod aspect
		 */
		const Aspect & getLodAspect() const;

		/**
		 * get modifiers for view
		 *
		 * @return  view modifiers
		 */
		const ModifierSet & getModifiers() const;

		/**
		 * get shader tag for view
		 *
		 * @return  shader tag
		 */
		const ShaderTag & getShaderTag() const;

		/**
		 * get uniforms for view
		 *
		 * @return  uniforms
		 */
		const UniformArray & getUniforms() const;

		/**
		 * get modifiers for view
		 *
		 * @return  view modifiers
		 */
		virtual bool isType(Type type) = 0;

		/**
		 * Check bounding box is visible by checking for intersection with camera
		 * frustum
		 *
		 * @param box
		 *            bounding box to test
		 * @param boxToWorld
		 *            transform to take box from local to world space
		 * @return true if box intersects frustum, false otherwise
		 */
		bool isVisible(const BoundingBox & box, const Transform & boxToWorld);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}
