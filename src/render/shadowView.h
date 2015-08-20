#pragma once

#include "view.h"

#include <string>

class Aspect;

namespace render {
	class ShadowView: public View {
	public:

		/**
		 * Constructor
		 *
		 * @param name       view name
		 * @param aspect     camera & transform
		 * @param lodAspect  aspect for calculating level of detail
		 * @param texture    target texture
		 * @param uniforms   view uniforms
		 * @param shader     view shader
		 * @param cull       cull flag
		 * @param type       view type
		 */
		ShadowView(const std::string & name, const Aspect & aspect,
				const Aspect & lodAspect,
				const std::shared_ptr<Texture> & texture, const Rect & rect,
				const UniformArray & uniforms, const ShaderTag & shader,
				View::Cull cull, View::DepthCompare depthCompare,
				const View::ModifierSet & modifiers);

		/**
		 * destructor
		 */
		~ShadowView();

		/**
		 * Do nothing
		 */
		void addAlphaIndexedTriangles(const RenderState &,
				const IndexedTriangles &, float);

		/**
		 * Do nothing
		 */
		void addDebugGeometry(const std::vector<DebugGeometry> &);

		/**
		 * Do nothing
		 */
		void addEdgeIndexArray(const RenderState &, const IndexArray &);

		/**
		 * Do nothing
		 */
		void addPoint(const RenderState &);

		/**
		 * Do nothing
		 */
		void addVolumetric(const RenderState &, const IndexedTriangles &);

		/**
		 * Do nothing
		 */
		void addSphere(const RenderState &, float);

		/**
		 *
		 *
		 * @param type
		 * @return
		 */
		bool isType(View::Type type) override;

		/**
		 * Prepare OpenGL state and render nodes
		 */
		void execute(Canvas & canvas) override;

		/**
		 * Add new indices node to list
		 */
		void addIndexedTriangles(const RenderState & state,
				const IndexedTriangles & indices);

		/**
		 *
		 * @return
		 */
		bool hasExecuted() const override;

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

