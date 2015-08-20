#include "sgVolumetricLighting.h"

#include "sgShader.h"

#include "../core/aspect.h"
#include "../core/perspectiveCamera.h"
#include "../core/vec2.h"
#include "../core/vec3.h"

#include "../render/indexedTriangles.h"
#include "../render/renderState.h"
#include "../render/shaderFlag.h"
#include "../render/textureManager.h"
#include "../render/uniform.h"
#include "../render/uniformArray.h"
#include "../render/vertexAttribute.h"
#include "../render/view.h"
#include "../render/viewBuilder.h"

#include "../scripting/executable.h"
#include "../scripting/parameters.h"

using namespace render;

namespace {
	std::vector<BaseParameter> params = {
			Parameter<Real>("near", nullptr),
			Parameter<Real>("far", nullptr),
			Parameter<Real>("slices", nullptr),
			Parameter<Real>("density", nullptr) };

	struct Factory: public Executable {
		Parameters parameters;

		Factory() :
				parameters(params) {
		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto args = parameters.getArgs(nArgs, stack);

			auto near =
					std::static_pointer_cast<Real>(args["near"])->getFloat();
			auto far = std::static_pointer_cast<Real>(args["far"])->getFloat();
			auto slices =
					std::static_pointer_cast<Real>(args["slices"])->getInt32();
			auto density =
					std::static_pointer_cast<Real>(args["density"])->getFloat();
			stack.push(
					std::make_shared<SgVolumetricLighting>(near, far, slices,
							density));
		}
	};

	// a_position attribute
	std::vector<Vec3> verts { Vec3(-1, -1, 0), Vec3(-1, 1, 0), Vec3(1, 1, 0),
			Vec3(1, -1, 0) };
	VertexAttribute a_position("a_position", verts);
	// a_offsetUV attribute
	std::vector<Vec2> uvs { Vec2(0, 0), Vec2(0, 1), Vec2(1, 1), Vec2(1, 0) };
	VertexAttribute a_offsetUV("a_offsetUV", uvs);
	// shader
	std::vector<std::string> flags = { "VERTEX_TRANSFORM", "COLOR", "SHADOW",
			"OFFSETZ" };
	SgShader shader(
			ShaderTag(nullptr, nullptr, nullptr, makeShaderFlags(flags)));
	// id for Scale uniform
	auto transformUID = Uniform::getUID("u_vertexTransform");
	// triangle indices
	std::vector<short> indicesFF { 0, 2, 1, 0, 3, 2 };
	IndexedTriangles trianglesFF = IndexedTriangles(IndexArray(indicesFF));
	std::vector<short> indicesBF { 0, 1, 2, 0, 2, 3 };
	IndexedTriangles trianglesBF = IndexedTriangles(IndexArray(indicesBF));
}

struct SgVolumetricLighting::impl {
	float m_near;
	float m_far;
	float m_step;
	UniformArray m_uniforms;

	impl(float near, float far, int slices, float density) :
					m_near(near),
					m_far(far),
					m_step((far - near) / (static_cast<float>(slices) - 1)) {
		m_uniforms.add(
				Uniform(Uniform::getUID("Color"), 1, 1, 1,
						density / static_cast<float>(slices)));
		m_uniforms.add(
				Uniform(Uniform::getUID("u_offsetMap"),
						TextureManager::getInstance().getImage("textures/",
								"random.png", false, true, true)));
		m_uniforms.add(Uniform(Uniform::getUID("u_offsetScale"), 2 * m_step));
	}
};

/**
 * constructor
 *
 * @param near
 * @param far
 * @param slices
 * @param density
 */
SgVolumetricLighting::SgVolumetricLighting(float near, float far, int slices,
		float density) :
		pimpl(new impl(near, far, slices, density)) {
}

/**
 * destructor
 */
SgVolumetricLighting::~SgVolumetricLighting(){
}

/**
 *
 * @param vb
 */
OVERRIDE void SgVolumetricLighting::visualize(render::ViewBuilder & vb) {
	const auto & view = vb.getView();
	if (view->isType(View::Type::REGULAR)) {
		const auto & aspect = view->getAspect();

		vb.pushState();
		auto & state = vb.getState();
		shader.visualize(vb);
		state.setBlend(BlendFlag::SRC_ALPHA_ADDITIVE);
		state.transform(aspect.getRotTrans());
		state.addUniforms(pimpl->m_uniforms);
		state.addVertexAttribute(a_position);
		state.addVertexAttribute(a_offsetUV);

		auto cam = std::static_pointer_cast<PerspectiveCamera>(
				aspect.getCamera());
		const auto & m = aspect.getTransform();
		auto det = m.determinant();
		const auto & triangles = det > 0 ? trianglesFF : trianglesBF;

		for (float z = -pimpl->m_far; z < -pimpl->m_near; z += pimpl->m_step) {
			vb.pushState();
			auto & state2 = vb.getState();
			float sy = -z * cam->getTanHalfFovy();
			float sx = cam->getAspectRatio() * sy;
			Mat4 transform(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, z, 0, 0, 0, 1);
			transform.mul(m, transform);
			state2.addUniform(Uniform(transformUID, transform));
			vb.addVolumetric(triangles);
			vb.popState();
		}

		vb.popState();
	}
}

/**
 * get script object factory for SgVolumetricLighting
 *
 * @return  SgVolumetricLighting factory
 */
STATIC ScriptObjectPtr SgVolumetricLighting::getFactory() {
	static auto factory = std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>());
	return factory;
}
