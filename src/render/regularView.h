#pragma once

#include "view.h"

#include <memory>

class Mat4;
class Transform;

namespace render {
	class RegularView: public View {
	public:

		RegularView(const std::string & name, const Aspect & aspect,
				const std::vector<std::shared_ptr<Texture>> & textures,
				const Rect & rect, const UniformArray & uniforms,
				const ShaderTag & shader, View::Cull cull,
				View::DepthCompare depthCompare,
				const View::ModifierSet & modifiers, int level);

		RegularView(const std::string & name, const Aspect & aspect,
				const std::shared_ptr<Texture> & texture, const Rect & rect,
				const UniformArray & uniforms, const ShaderTag & shader,
				View::Cull cull, View::DepthCompare depthCompare,
				const View::ModifierSet & modifiers, int level);

		/**
		 * destructor
		 */
		~RegularView();

		void addAlphaIndexedTriangles(const RenderState & state,
				const IndexedTriangles & indices, float z) override;

		void addDebugGeometry(const std::vector<DebugGeometry> & debug)
				override;

		void addEdgeIndexArray(const RenderState & state,
				const IndexArray & indices) override;

		void addIndexedTriangles(const RenderState & state,
				const IndexedTriangles & indices) override;

		void addPoint(const RenderState & state) override;

		void addSphere(const RenderState & state, float radius) override;

		/**
		 *
		 */
		void addVolumetric(const RenderState & state,
				const IndexedTriangles & indices) override;

		/**
		 * Execute drawing of view
		 */
		void execute(Canvas & canvas) override;

		bool hasExecuted() const override;

		bool isType(View::Type type) override;

	private:
		/**
		 * Draw nodes
		 *
		 * @param canvas
		 * @param projMatrix
		 * @param worldToView
		 * @param globalUniforms
		 */
		void executeNodes(Canvas & canvas, const Mat4 & projMatrix,
				const Transform & worldToView,
				const UniformArray & globalUniforms);

		/**
		 * volumetric nodes
		 */
		void executeVolumetric(Canvas & canvas, const Transform & worldToView,
				const UniformArray & globalUniforms);

		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

