#include "shadowView.h"

#include "canvas.h"
#include "glDebug.h"
#include "indexedTriangles.h"
#include "renderState.h"
#include "shaderManager.h"
#include "uniform.h"
#include "uniformArray.h"

#include "../core/aspect.h"
#include "../core/color.h"
#include "../core/transform.h"

#include <unordered_map>

#include <GL/glew.h>

using namespace render;

namespace {
	auto rgbaToFloatUID = Uniform::getUID("rgbaToFloat");

	auto worldView_matrixUID = Uniform::getUID("worldView_matrix");
	auto proj_matrixUID = Uniform::getUID("proj_matrix");

	struct Node {
		RenderState state;
		IndexedTriangles triangles;
		std::vector<Transform> transforms;
		size_t maxInstances;

		Node(const RenderState & state, const IndexedTriangles & triangles,
				int maxInstances) :
				state(state), triangles(triangles), maxInstances(maxInstances) {
		}

		void addInstance(const Transform & transform) {
			transforms.emplace_back(transform);
		}

		void execute(const Mat4 & worldToViewMatrix,
				const UniformArray & globalUniforms) {
			state.addUniforms(globalUniforms);

			for (size_t i = 0, n = transforms.size(); i < n; i += maxInstances) {
				auto nInstances = n - i < maxInstances ? n - i : maxInstances;

				for (size_t j = 0; j < nInstances; ++j) {
					Mat4 modelMatrix = transforms.at(i + j).asMat4();

					Mat4 modelViewMatrix;
					modelViewMatrix.mul(worldToViewMatrix, modelMatrix);

					// model to view matrix
					auto uid = Uniform::getUID(
							"modelView_matrices[" + std::to_string(j) + "]");
					state.addUniform(Uniform(uid, modelViewMatrix));
				}

				// world to view matrix
				state.addUniform(
						Uniform(worldView_matrixUID, worldToViewMatrix));

				state.bindUniforms();
				state.bindVertexAttributes();

				triangles.drawInstances(nInstances);
			}
		}
	};

	/**
	 * Convert cull flags to OpenGL
	 *
	 * @param c  flag to convert
	 * @return   OpenGL flag
	 */
	int convertCullFlag(View::Cull c) {
		switch (c) {
		case View::Cull::NONE:
			return GL_NONE;
		case View::Cull::BACK:
			return GL_BACK;
		case View::Cull::FRONT:
			return GL_FRONT;
		case View::Cull::FRONTBACK:
			return GL_FRONT_AND_BACK;
		}
		return GL_BACK;
	}
}

struct ShadowView::impl {
	/** nodes to render */
	std::unordered_map<unsigned, std::vector<Node>> nodesByShader;
	bool executed = false;
};

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
ShadowView::ShadowView(const std::string & name, const Aspect & aspect,
		const Aspect & lodAspect, const std::shared_ptr<Texture> & texture,
		const Rect & rect, const UniformArray & uniforms,
		const ShaderTag & shader, View::Cull cull,
		View::DepthCompare depthCompare, const View::ModifierSet & modifiers) :
				View(name, aspect, lodAspect, texture, rect, uniforms, shader,
						cull, depthCompare, modifiers, 0),
						pimpl(new impl()) {
}

/**
 * destructor
 */
ShadowView::~ShadowView() {
}

/**
 * Do nothing
 */
void ShadowView::addAlphaIndexedTriangles(const RenderState &,
		const IndexedTriangles &, float) {
}

/**
 * Do nothing
 */
void ShadowView::addDebugGeometry(const std::vector<DebugGeometry> &) {
}

/**
 * Do nothing
 */
void ShadowView::addEdgeIndexArray(const RenderState &, const IndexArray &) {
}

/**
 * Add new indices node to list
 */
void ShadowView::addIndexedTriangles(const RenderState & state,
		const IndexedTriangles & indices) {
	addDependencies(state.getTextureDependencies());

	auto shaderId = state.getShaderId();

	auto & nodes = pimpl->nodesByShader[shaderId];

	for (auto & node : nodes) {
		if (indices == node.triangles && state.isInstanceable(node.state)) {
			node.addInstance(state.getTransform());
			return;
		}
	}

	auto shader = ShaderManager::getInstance().getShader(shaderId);
	Node node(state, indices, shader->getMaxInstances());
	node.addInstance(state.getTransform());
	nodes.emplace_back(node);
}

/**
 * Do nothing
 */
void ShadowView::addPoint(const RenderState &) {
}

/**
 * Do nothing
 */
void ShadowView::addSphere(const RenderState &, float) {
}

/**
 * Do nothing
 */
void ShadowView::addVolumetric(const RenderState &, const IndexedTriangles &) {
}

/**
 * Prepare OpenGL state and render nodes
 */
OVERRIDE void ShadowView::execute(Canvas & canvas) {
	if (canvas.setTarget(getTargetTextures()) == false) {
		return;
	}

	Rect rect = getViewport();
	int width = (int) rect.getWidth();
	int height = (int) rect.getHeight();
	glViewport((int) rect.getLeft(), (int) rect.getBottom(), width, height);

	// clear
	if (getOpaque()) {
		const auto & bg = getBGColor();
		glClearColor(bg.getR(), bg.getG(), bg.getB(), 0);
		switch (getDepthCompare()) {
		case View::DepthCompare::LESS:
			glClearDepth(1);
			break;
		case View::DepthCompare::GREATER:
			glClearDepth(0);
			break;
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	const auto & camera = getAspect().getCamera();

	UniformArray globalUniforms;
	// pack float into rgba
	int r, g, b;
	glGetIntegerv(GL_RED_BITS, &r);
	glGetIntegerv(GL_GREEN_BITS, &g);
	glGetIntegerv(GL_BLUE_BITS, &b);
	globalUniforms.add(
			Uniform(rgbaToFloatUID, static_cast<float>(1 << r),
					static_cast<float>(1 << g), static_cast<float>(1 << b)));

	// enable
	int oglCull = convertCullFlag(getCull());
	if (oglCull != GL_NONE) {
		glCullFace(oglCull);
		glEnable(GL_CULL_FACE);
	}
	// depth test
	glEnable(GL_DEPTH_TEST);
	switch (getDepthCompare()) {
	case View::DepthCompare::LESS:
		glDepthFunc(GL_LESS);
		break;
	case View::DepthCompare::GREATER:
		glDepthFunc(GL_GREATER);
		break;
	}

	// view matrix
	Transform worldToView = getAspect().getRotTrans().inverse();
	Mat4 worldToViewMatrix = worldToView.asMat4();

	// view to proj matrix
	globalUniforms.add(Uniform(proj_matrixUID, camera->getProjectionMatrix()));

	// execute nodes
	for (auto & e : pimpl->nodesByShader) {
		// bind shader
		auto shader = ShaderManager::getInstance().getShader(e.first);
		shader->bind();

		for (auto & node : e.second) {
			node.execute(worldToViewMatrix, globalUniforms);
		}

		shader->unbind();
	}

	// disable
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	GlDebug::printOpenGLError();

	pimpl->executed = true;
}

/**
 *
 * @return
 */
OVERRIDE bool ShadowView::hasExecuted() const {
	return pimpl->executed;
}

/**
 *
 *
 * @param type
 * @return
 */
OVERRIDE bool ShadowView::isType(View::Type type) {
	return type == View::Type::SHADOW;
}
