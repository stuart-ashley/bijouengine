#include "regularView.h"

#include "canvas.h"
#include "glDebug.h"
#include "indexedTriangles.h"
#include "renderState.h"
#include "shaderManager.h"
#include "textureManager.h"
#include "uniform.h"
#include "uniformArray.h"

#include "../core/aspect.h"
#include "../core/debugGeometry.h"
#include "../core/color.h"
#include "../core/mat3.h"
#include "../core/mat4.h"
#include "../core/transform.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

#include <GL/glew.h>

using namespace render;

namespace {

	auto rgbaToFloatUID = Uniform::getUID("rgbaToFloat");
	auto cameraPositionUID = Uniform::getUID("u_cameraPosition");

	auto proj_matrixUID = Uniform::getUID("proj_matrix");
	auto mvp_matrixUID = Uniform::getUID("mvp_matrix");
	auto modelView_matricesUID = Uniform::getUID("modelView_matrices");
	auto colorUID = Uniform::getUID("color");
	auto color0UID = Uniform::getUID("u_color0");
	auto color1UID = Uniform::getUID("u_color1");

	size_t clipPlaneUIDs[] = { Uniform::getUID("clipPlane0"), Uniform::getUID(
			"clipPlane1"), Uniform::getUID("clipPlane2"), Uniform::getUID(
			"clipPlane3") };

	auto centreUID = Uniform::getUID("u_centre");
	auto radiusUID = Uniform::getUID("u_radius");
	auto localMatrixUID = Uniform::getUID("u_localMatrix");

	float clipSpaceFullscreenVerts[] =
			{ -1, -1, 0, -1, 1, 0, 1, 1, 0, 1, -1, 0 };

	/**
	 * Render node for indexed triangles
	 */
	struct IdxTrisNode {
		RenderState state;
		IndexedTriangles triangles;
		std::vector<Transform> transforms;
		size_t maxInstances;

		IdxTrisNode(const RenderState & state,
				const IndexedTriangles & triangles, int capacity) :
				state(state), triangles(triangles), maxInstances(capacity) {
		}

		void addInstance(const Transform & transform) {
			transforms.emplace_back(transform);
		}

		void execute(const Transform & worldToView,
				const UniformArray & globalUniforms) {
			state.addUniforms(globalUniforms);

			for (size_t i = 0, n = transforms.size(); i < n; i +=
					maxInstances) {
				auto nInstances = n - i < maxInstances ? n - i : maxInstances;

				for (size_t j = 0; j < nInstances; ++j) {
					Transform modelToView = worldToView;
					modelToView.transform(transforms[i + j]);
					Mat4 modelViewMatrix = modelToView.asMat4();

					// TODO normal matrix is inverse transpose model view matrix
					Mat3 normalMatrix = modelViewMatrix.getRotationScale();

					state.addUniform(
							Uniform(
									Uniform::getUID(
											"modelView_matrices["
													+ std::to_string(j) + "]"),
									modelViewMatrix));
					state.addUniform(
							Uniform(
									Uniform::getUID(
											"normal_matrices["
													+ std::to_string(j) + "]"),
									normalMatrix));
				}

				state.bindUniforms();
				state.bindVertexAttributes();

				triangles.drawInstances(nInstances);
				GlDebug::printOpenGLError();
			}
		}
	};

	/**
	 * Render node for points
	 */
	struct PointNode {
		RenderState state;
		std::vector<Vec3> points;

		PointNode(const RenderState & state) :
				state(state) {
		}

		void addPoint(const Vec3 & point) {
			points.emplace_back(point);
		}

		void execute(UniformArray uniforms) {
			// bind shader & uniforms
			state.bindShader();
			state.addUniforms(uniforms);
			state.bindUniforms();

			int shaderId = state.getShaderId();
			const auto & shader = ShaderManager::getInstance().getShader(
					shaderId);
			int posIdx = shader->getAttributeIndex("a_position");

			// blend
			switch (state.getBlend()) {
			case BlendFlag::SRC_ALPHA_ADDITIVE:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
			case BlendFlag::SRC_ALPHA:
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			default:
				assert(false);
			}

			assert(sizeof(Vec3) == 12); // ensure Vec3 is 3 floats
			glVertexAttribPointer(posIdx, 3, GL_FLOAT, false, 0, points.data());
			glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(points.size()));

			// disable attributes
			state.unbindShader();
		}
	};

	/**
	 * Render node for spheres
	 */
	struct SphereNode {
		RenderState state;
		Vec3 centre;
		float radius;

		SphereNode(const RenderState & state, const Vec3 & centre, float radius) :
				state(state), centre(centre), radius(radius) {
		}

		void execute(const Aspect & aspect) {
			UniformArray uniforms;
			uniforms.add(Uniform(centreUID, centre));
			uniforms.add(Uniform(radiusUID, radius));
			Mat4 localMatrix =
					aspect.getRotTrans().to(state.getTransform()).asMat4();
			uniforms.add(Uniform(localMatrixUID, localMatrix));

			state.bindShader();
			state.addUniforms(uniforms);
			state.bindUniforms();

			int shaderId = state.getShaderId();
			const auto & shader = ShaderManager::getInstance().getShader(
					shaderId);
			int posIdx = shader->getAttributeIndex("a_position");

			glDisable(GL_CULL_FACE);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glVertexAttribPointer(posIdx, 3, GL_FLOAT, false, 0,
					clipSpaceFullscreenVerts);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

			glDisable(GL_BLEND);
			glEnable(GL_CULL_FACE);

			// disable attributes
			shader->unbind();
		}
	};

	/**
	 * Render node for indexed edges
	 */
	struct IdxEdgeNode {
		RenderState state;
		IndexArray indices;

		IdxEdgeNode(const RenderState & state, const IndexArray & indices) :
				state(state), indices(indices) {
		}

		void execute(const Transform & worldToView, const Mat4 & projMatrix,
				const UniformArray & globalUniforms) {
			state.bindShader();

			auto modelToView = worldToView;
			modelToView.transform(state.getTransform());

			auto modelViewMatrix = modelToView.asMat4();

			Mat4 modelViewProjMatrix;
			modelViewProjMatrix.mul(projMatrix, modelViewMatrix);

			state.addUniforms(globalUniforms);
			state.addUniform(Uniform(mvp_matrixUID, modelViewProjMatrix));
			state.addUniform(Uniform(modelView_matricesUID, modelViewMatrix));

			state.bindUniforms();
			state.bindVertexAttributes();

			glDrawElements(GL_LINES, static_cast<GLsizei>(indices.size()),
					GL_UNSIGNED_SHORT, indices.getBuffer());
			GlDebug::printOpenGLError();

			// disable attributes
			state.unbindShader();
		}
	};

	/**
	 * Render node for points & lines
	 */
	struct DebugNode {
		DebugGeometry dg;
		UniformArray uniforms;

		static void draw(const std::vector<Vec3> & array, int type,
				int posIdx) {
			static const int maxVertices = 10000;
			static std::array<float, maxVertices * 3> buffer;
			for (size_t i = 0, n = array.size(); i < n; i += maxVertices) {
				size_t count = n - i;
				for (size_t j = 0; j < count; ++j) {
					buffer[j * 3] = static_cast<float>(array[i + j].getX());
					buffer[j * 3 + 1] = static_cast<float>(array[i + j].getY());
					buffer[j * 3 + 2] = static_cast<float>(array[i + j].getZ());
				}

				glVertexAttribPointer(posIdx, 3, GL_FLOAT, false, 0,
						buffer.data());
				glDrawArrays(type, 0, static_cast<GLsizei>(count));
			}
		}

		DebugNode(const DebugGeometry & debugGeometry) :
				dg(debugGeometry) {
			uniforms.add(Uniform(colorUID, dg.getColor()));
			uniforms.add(Uniform(color0UID, dg.getColor()));
			uniforms.add(Uniform(color1UID, dg.getEndColor()));
		}

		void execute(const Transform & worldToView, const Mat4 & projMatrix) {
			auto modelToView = worldToView;
			modelToView.transform(dg.getTransform());

			auto modelViewProjMatrix = modelToView.asMat4();
			modelViewProjMatrix.mul(projMatrix, modelViewProjMatrix);

			uniforms.add(Uniform(mvp_matrixUID, modelViewProjMatrix));

			if (dg.getLines().size() > 0) {
				const auto & shader =
						ShaderManager::getInstance().getDebugLinesShader();
				shader->bind();
				int posIdx = shader->getAttributeIndex("a_position");
				uniforms.bind(shader);
				draw(dg.getLines(), GL_LINES, posIdx);
				shader->unbind();
			}

			if (dg.getPoints().size() > 0) {
				const auto & shader =
						ShaderManager::getInstance().getDebugShader();
				shader->bind();
				int posIdx = shader->getAttributeIndex("a_position");
				uniforms.bind(shader);
				draw(dg.getPoints(), GL_POINTS, posIdx);
				shader->unbind();
			}
		}
	};

	void clearFace(const Color & bgColor) {

		const auto & shader = ShaderManager::getInstance().getDebugShader();
		shader->bind();

		UniformArray uniforms;
		uniforms.add(Uniform(colorUID, bgColor));
		uniforms.add(Uniform(mvp_matrixUID, Mat4::identity()));
		uniforms.bind(shader);

		int posIdx = shader->getAttributeIndex("a_position");

		glClear(GL_DEPTH_BUFFER_BIT);
		glDepthMask(false);
		glVertexAttribPointer(posIdx, 3, GL_FLOAT, false, 0,
				clipSpaceFullscreenVerts);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDepthMask(true);

		// disable attributes
		shader->unbind();
	}
}

struct RegularView::impl {
	std::vector<std::vector<IdxTrisNode>> idxTrisNodesByShader;
	std::array<std::vector<std::vector<IdxTrisNode>>, 16> alphaDepthBuckets;
	std::array<std::vector<std::vector<PointNode>>, 16> pointBuckets;
	std::vector<SphereNode> spheresNodes;
	std::vector<IdxEdgeNode> idxEdgeNodes;
	std::vector<DebugNode> debugNodes;
	std::vector<IdxTrisNode> volumetricNodes;
	bool executed;

	impl() :
			executed(false) {
	}

	void drawPoints(const UniformArray & globalUniforms) {
		UniformArray uniforms = globalUniforms;

		glDepthMask(false);
		glEnable(GL_BLEND);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		for (auto & bucket : pointBuckets) {
			for (auto & nodesForShader : bucket) {
				if (nodesForShader.size() == 0) {
					continue;
				}

				for (auto & node : nodesForShader) {
					node.execute(uniforms);
				}
			}
		}
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_BLEND);
		glDepthMask(true);
	}
};

RegularView::RegularView(const std::string & name, const Aspect & aspect,
		const std::vector<std::shared_ptr<Texture>> & textures,
		const Rect & rect, const UniformArray & uniforms,
		const ShaderTag & shader, View::Cull cull,
		View::DepthCompare depthCompare, const View::ModifierSet & modifiers,
		int level) :
				View(name, aspect, aspect, textures, rect, uniforms, shader,
						cull, depthCompare, modifiers, level),
				pimpl(new impl()) {
}

RegularView::RegularView(const std::string & name, const Aspect & aspect,
		const std::shared_ptr<Texture> & texture, const Rect & rect,
		const UniformArray & uniforms, const ShaderTag & shader,
		View::Cull cull, View::DepthCompare depthCompare,
		const View::ModifierSet & modifiers, int level) :
				View(name, aspect, aspect, texture, rect, uniforms, shader,
						cull, depthCompare, modifiers, level),
				pimpl(new impl()) {
}

/**
 * destructor
 */
RegularView::~RegularView() {
}

OVERRIDE void RegularView::addAlphaIndexedTriangles(const RenderState & state,
		const IndexedTriangles & indices, float z) {
	addDependencies(state.getTextureDependencies());

	auto shaderId = state.getShaderId();

	size_t index = 0;
	if (z > .0625f) {
		index = (int) (std::log(z) / std::log(2) + 4);
		index = std::max(static_cast<size_t>(0),
				std::min(pimpl->alphaDepthBuckets.size() - 1, index));
	}
	auto & bucket = pimpl->alphaDepthBuckets.at(index);

	while (shaderId >= bucket.size()) {
		bucket.emplace_back();
	}

	auto & nodes = bucket.at(shaderId);

	for (auto & node : nodes) {
		if (indices == node.triangles && state.isInstanceable(node.state)) {
			node.addInstance(state.getTransform());
			return;
		}
	}

	auto shader = ShaderManager::getInstance().getShader(shaderId);
	IdxTrisNode node(state, indices, shader->getMaxInstances());
	node.addInstance(state.getTransform());
	nodes.emplace_back(node);
}

OVERRIDE void RegularView::addDebugGeometry(
		const std::vector<DebugGeometry> & debug) {
	pimpl->debugNodes.insert(pimpl->debugNodes.end(), debug.begin(),
			debug.end());
}

OVERRIDE void RegularView::addEdgeIndexArray(const RenderState & state,
		const IndexArray & indices) {
	addDependencies(state.getTextureDependencies());
	pimpl->idxEdgeNodes.emplace_back(state, indices);
}

OVERRIDE void RegularView::addIndexedTriangles(const RenderState & state,
		const IndexedTriangles & indices) {
	addDependencies(state.getTextureDependencies());

	auto shaderId = state.getShaderId();

	while (shaderId >= pimpl->idxTrisNodesByShader.size()) {
		pimpl->idxTrisNodesByShader.emplace_back();
	}

	auto & nodes = pimpl->idxTrisNodesByShader.at(shaderId);

	for (auto & node : nodes) {
		if (indices == node.triangles && state.isInstanceable(node.state)) {
			node.addInstance(state.getTransform());
			return;
		}
	}

	const auto & shader = ShaderManager::getInstance().getShader(shaderId);
	IdxTrisNode node(state, indices, shader->getMaxInstances());
	node.addInstance(state.getTransform());
	nodes.emplace_back(node);
}

OVERRIDE void RegularView::addPoint(const RenderState & state) {
	addDependencies(state.getTextureDependencies());

	auto shaderId = state.getShaderId();

	auto point = state.getTranslation();
	getAspect().getRotTrans().inverseTransformPoint(point);

	size_t index = 0;
	if (-point.getZ() > .0625f) {
		index = static_cast<int>(std::log(-point.getZ()) / std::log(2) + 4);
		index = std::max(static_cast<size_t>(0),
				std::min(pimpl->pointBuckets.size() - 1, index));
	}
	auto & bucket = pimpl->pointBuckets.at(index);

	while (shaderId >= bucket.size()) {
		bucket.emplace_back();
	}

	auto & nodes = bucket.at(shaderId);

	for (auto & node : nodes) {
		if (state.isInstanceable(node.state)) {
			node.addPoint(point);
			return;
		}
	}

	PointNode node(state);
	node.addPoint(point);
	nodes.emplace_back(node);
}

OVERRIDE void RegularView::addSphere(const RenderState & state, float radius) {
	addDependencies(state.getTextureDependencies());

	auto centre = state.getTranslation();
	getAspect().getRotTrans().inverseTransformPoint(centre);

	pimpl->spheresNodes.emplace_back(state, centre, radius);
}

/**
 *
 */
OVERRIDE void RegularView::addVolumetric(const RenderState & state,
		const IndexedTriangles & indices) {
	IdxTrisNode node(state, indices, 1);
	node.addInstance(state.getTransform());
	pimpl->volumetricNodes.emplace_back(node);
}

/**
 * Execute drawing of view
 */
OVERRIDE void RegularView::execute(Canvas & canvas) {
	if (canvas.setTarget(getTargetTextures()) == false) {
		return;
	}

	Rect rect = getViewport();
	int width = (int) rect.getWidth();
	int height = (int) rect.getHeight();
	glViewport((int) rect.getLeft(), (int) rect.getBottom(), width, height);

	// scissor
	glEnable(GL_SCISSOR_TEST);
	Rect scissor = getScissor();
	glScissor((int) scissor.getLeft(), (int) scissor.getBottom(),
			(int) scissor.getWidth(), (int) scissor.getHeight());

	// clear
	if (getOpaque()) {
		if (isCubeFace()) {
			// on ATI hardware GL_COLOR_BUFFER_BIT seems to clear the entire
			// cube map, not just the face attached to the FBO
			clearFace(getBGColor());
		} else {
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
	}

	const auto & aspect = getAspect();
	const auto & camera = aspect.getCamera();

	// world to view transform
	auto worldToView = aspect.getRotTrans().inverse();

	// clip planes
	UniformArray globalUniforms;
	const auto & planes = aspect.getClipPlanes();
	for (size_t i = 0, n = planes.size(); i < n; ++i) {
		auto p = planes[i];
		p.transform(worldToView);
		const auto & normal = p.getNormal();
		globalUniforms.add(
				Uniform(clipPlaneUIDs[i], normal.getX(), normal.getY(),
						normal.getZ(),
						static_cast<float>(-p.getPoint().dot(normal))));
	}

	// camera position
	Vec3 cameraPosition = aspect.getRotTrans().getTranslation();
	globalUniforms.add(Uniform(cameraPositionUID, cameraPosition));
	// camera projection matrix
	globalUniforms.add(Uniform(proj_matrixUID, camera->getProjectionMatrix()));

	// pack float into rgba
	int r, g, b;
	glGetIntegerv(GL_RED_BITS, &r);
	glGetIntegerv(GL_GREEN_BITS, &g);
	glGetIntegerv(GL_BLUE_BITS, &b);
	globalUniforms.add(
			Uniform(rgbaToFloatUID, static_cast<float>(1 << r),
					static_cast<float>(1 << g), static_cast<float>(1 << b)));

	// find winding order and select culling accordingly
	glEnable(GL_CULL_FACE);
	float det = camera->getProjectionMatrix().determinant();
	if (det > 0) {
		glCullFace(GL_FRONT);
	} else {
		glCullFace(GL_BACK);
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

	// execute nodes
	executeNodes(canvas, camera->getProjectionMatrix(), worldToView,
			globalUniforms);

	// disable
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);

	pimpl->executed = true;
}

/**
 * Draw nodes
 *
 * @param projMatrix
 * @param worldToView
 * @param globalUniforms
 */
PRIVATE void RegularView::executeNodes(Canvas & canvas, const Mat4 & projMatrix,
		const Transform & worldToView, const UniformArray & globalUniforms) {
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0);
	for (auto & nodesForShader : pimpl->idxTrisNodesByShader) {
		if (nodesForShader.size() == 0) {
			continue;
		}
		// bind shader
		auto & state = nodesForShader.at(0).state;
		state.bindShader();

		for (auto & node : nodesForShader) {
			node.execute(worldToView, globalUniforms);
		}

		// disable attributes
		state.unbindShader();
	}
	glDisable(GL_ALPHA_TEST);

	glDepthMask(false);
	glEnable(GL_BLEND);
	BlendFlag currentBlend = BlendFlag::NONE;
	for (auto & bucket : pimpl->alphaDepthBuckets) {
		for (auto & nodesForShader : bucket) {
			if (nodesForShader.size() == 0) {
				continue;
			}
			// bind shader
			auto & state = nodesForShader.at(0).state;
			state.bindShader();

			for (auto & node : nodesForShader) {
				BlendFlag blend = node.state.getBlend();
				if (blend != currentBlend) {
					currentBlend = blend;
					switch (blend) {
					case BlendFlag::SRC_ALPHA_ADDITIVE:
						glBlendFunc(GL_SRC_ALPHA, GL_ONE);
						break;
					case BlendFlag::SRC_ALPHA:
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						break;
					default:
						assert(false);
						continue;
					}
				}
				node.execute(worldToView, globalUniforms);
			}

			// disable attributes
			state.unbindShader();
		}
	}
	glDisable(GL_BLEND);
	glDepthMask(true);

	pimpl->drawPoints(globalUniforms);

	// sort sphere nodes by distance
	std::sort(pimpl->spheresNodes.begin(), pimpl->spheresNodes.end(),
			[](const SphereNode & a, const SphereNode & b) {
				return a.centre.getZ() < b.centre.getZ();
			});
	for (auto & node : pimpl->spheresNodes) {
		node.execute(getAspect());
	}

	for (auto & node : pimpl->idxEdgeNodes) {
		node.execute(worldToView, projMatrix, globalUniforms);
	}

	for (auto & node : pimpl->debugNodes) {
		node.execute(worldToView, projMatrix);
	}

	// volumetric
	if (pimpl->volumetricNodes.size() > 0) {
		executeVolumetric(canvas, worldToView, globalUniforms);
	}
}

/**
 * volumetric nodes
 */
PRIVATE void RegularView::executeVolumetric(Canvas & canvas,
		const Transform & worldToView, const UniformArray & globalUniforms) {
	// no culling
	glDisable(GL_CULL_FACE);
	// disable depth
	glDepthMask(false);
	// enable blend
	glEnable(GL_BLEND);

	/*
	 * draw nodes to temporary texture
	 */
	// set target
	int width = (int) getViewport().getWidth();
	int height = (int) getViewport().getHeight();
	const auto & volTexture = TextureManager::getInstance().getTexture(
			"regularViewTemporaryVolumetricTexture", width, height, true);
	canvas.setTarget(volTexture);
	// clear colors
	glClear(GL_COLOR_BUFFER_BIT);
	// blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// bind shader
	auto & state = pimpl->volumetricNodes.at(0).state;
	state.bindShader();
	// execute volumetric nodes
	for (auto & node : pimpl->volumetricNodes) {
		node.execute(worldToView, globalUniforms);
	}

	// disable attributes
	state.unbindShader();

	/*
	 * horizontal volumetric texture blur to temporary texture
	 */

	// reset target
	const auto & blurTexture = TextureManager::getInstance().getTexture(
			"regularViewTemporaryBlurTexture", width, height, true);
	canvas.setTarget(blurTexture);
	// clear colors
	glClear(GL_COLOR_BUFFER_BIT);

	// setup quad
	float uvxy[] = { 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0 };

	// blend function
	glBlendFunc(GL_ONE, GL_ONE);

	// horizontal blur shader
	const auto & horizBlur =
			ShaderManager::getInstance().getHorizontalBlurShader();
	horizBlur->bind();

	// setup uniforms
	UniformArray uforms;
	uforms.add(
			Uniform(mvp_matrixUID,
					Mat4(2, 0, 0, -1, 0, 2, 0, -1, 0, 0, -1, 0, 0, 0, 0, 1)));
	uforms.add(Uniform(colorUID, 1, 1, 1, 1));
	uforms.add(Uniform(Uniform::getUID("Source"), volTexture));
	uforms.add(Uniform(Uniform::getUID("u_blurSize"), 1.f));
	uforms.add(
			Uniform(Uniform::getUID("SrcDimensions"), static_cast<float>(width),
					static_cast<float>(height)));
	uforms.bind(horizBlur);

	// get attribute indices
	int posIdx = horizBlur->getAttributeIndex("a_position");
	int uvIdx = horizBlur->getAttributeIndex("a_uv");

	// setup attributes
	glVertexAttribPointer(posIdx, 2, GL_FLOAT, false, 4 * sizeof(float),
			uvxy + 2);
	glVertexAttribPointer(uvIdx, 2, GL_FLOAT, false, 4 * sizeof(float), uvxy);

	// draw quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// disable attributes
	horizBlur->unbind();

	/*
	 * vertical volumetric blur to view target
	 */

	// reset target
	canvas.setTarget(getTargetTextures());

	// vertical blur vertBlur
	const auto & vertBlur =
			ShaderManager::getInstance().getVerticalBlurShader();
	vertBlur->bind();

	// setup uniforms
	uforms.add(Uniform(Uniform::getUID("Source"), blurTexture));
	uforms.bind(vertBlur);

	// get attribute indices
	posIdx = vertBlur->getAttributeIndex("a_position");
	uvIdx = vertBlur->getAttributeIndex("a_uv");

	// setup attributes
	glVertexAttribPointer(posIdx, 2, GL_FLOAT, false, 4 * sizeof(float),
			uvxy + 2);
	glVertexAttribPointer(uvIdx, 2, GL_FLOAT, false, 4 * sizeof(float), uvxy);

	// draw quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// disable attributes
	vertBlur->unbind();

	// disable blend
	glDisable(GL_BLEND);

	// enable depth
	glDepthMask(true);
}

OVERRIDE bool RegularView::hasExecuted() const {
	return pimpl->executed;
}

OVERRIDE bool RegularView::isType(View::Type type) {
	return type == View::Type::REGULAR;
}
