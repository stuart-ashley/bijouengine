#include "renderState.h"

#include "blendFlag.h"
#include "lighting.h"
#include "shaderFlag.h"
#include "shaderManager.h"
#include "shaderTag.h"
#include "texture.h"
#include "uniform.h"
#include "uniformArray.h"
#include "vertexAttribute.h"
#include "vertexAttributeArray.h"

#include "../core/boundingBox.h"
#include "../core/quat.h"
#include "../core/transform.h"
#include "../core/vec3.h"

#include <cassert>
#include <array>

using namespace render;

namespace {

	auto LightMapUID = Uniform::getUID("LightMap");
	auto LumaMapUID = Uniform::getUID("LumaMap");

	auto lightMapFlag = ShaderFlag::valueOf("LIGHTMAP");
	auto lumaMapFlag = ShaderFlag::valueOf("LUMAMAP");

	template<typename TYPE>
	inline void makeUnique(std::shared_ptr<TYPE> & ptr) {
		if (ptr.unique() == false) {
			ptr = std::make_shared<TYPE>(*ptr);
		}
	}

	template<typename TYPE>
	inline void makeUnique2(std::shared_ptr<TYPE> & ptr) {
		if (ptr == nullptr) {
			ptr = ptr = std::make_shared<TYPE>();
		} else if (ptr.unique() == false) {
			ptr = std::make_shared<TYPE>(*ptr);
		}
	}
}

struct RenderState::impl {

	std::shared_ptr<Transform> m_transform;
	std::shared_ptr<ShaderTag> m_tag;
	std::shared_ptr<Shader> m_shader;
	BlendFlag m_blend;
	std::shared_ptr<UniformArray> m_uniforms;
	std::shared_ptr<VertexAttributeArray> m_vertexAttributes;
	std::shared_ptr<std::array<Mat4, 32>> m_skinningMatrices;
	std::shared_ptr<Lighting> m_lighting;

	impl(const Lighting & lighting, const ShaderTag & tag) :
					m_transform(std::make_shared<Transform>()),
					m_tag(std::make_shared<ShaderTag>(tag)),
					m_shader(nullptr),
					m_blend(BlendFlag::NONE),
					m_uniforms(nullptr),
					m_vertexAttributes(nullptr),
					m_skinningMatrices(nullptr),
					m_lighting(std::make_shared<Lighting>(lighting)) {
	}

	impl(const impl & other) :
					m_transform(other.m_transform),
					m_tag(other.m_tag),
					m_shader(other.m_shader),
					m_blend(other.m_blend),
					m_uniforms(other.m_uniforms),
					m_vertexAttributes(other.m_vertexAttributes),
					m_skinningMatrices(other.m_skinningMatrices),
					m_lighting(other.m_lighting) {
	}

};

/*
 *
 */
RenderState::RenderState(const Lighting & lighting, const ShaderTag & tag) :
		pimpl(std::make_shared<impl>(lighting, tag)) {
}

/*
 *
 */
void RenderState::addShaderFlag(size_t flag) {
	makeUnique(pimpl);
	makeUnique(pimpl->m_tag);

	pimpl->m_tag->addFlag(flag);
}

/*
 *
 */
void RenderState::addShaderFlags(const ShaderFlags & flags) {
	makeUnique(pimpl);
	makeUnique(pimpl->m_tag);

	pimpl->m_tag->addFlags(flags);
}

/*
 *
 */
void RenderState::addUniform(const Uniform & uniform) {
	makeUnique(pimpl);
	makeUnique2(pimpl->m_uniforms);

	pimpl->m_uniforms->add(uniform);
}

/*
 *
 */
void RenderState::addUniforms(const UniformArray & uniforms) {
	makeUnique(pimpl);
	makeUnique2(pimpl->m_uniforms);

	pimpl->m_uniforms->add(uniforms);
}

/*
 *
 */
void RenderState::addVertexAttribute(const VertexAttribute & attribute) {
	makeUnique(pimpl);
	makeUnique2(pimpl->m_vertexAttributes);

	pimpl->m_vertexAttributes->add(attribute);
}

/*
 *
 */
void RenderState::bindShader() const {
	pimpl->m_shader->bind();
}

/*
 *
 */
void RenderState::bindUniforms() const {
	pimpl->m_uniforms->bind(pimpl->m_shader);
}

/*
 *
 */
void RenderState::bindVertexAttributes() const {
	pimpl->m_vertexAttributes->bind(pimpl->m_shader);
}

/*
 *
 */
void RenderState::clipLights(const BoundingBox & bounds) {
	makeUnique(pimpl);

	// clone lighting, making clipping local to this state
	pimpl->m_lighting = std::shared_ptr<Lighting>(pimpl->m_lighting);
	pimpl->m_lighting->clipLights(bounds, *pimpl->m_transform);
}

/*
 *
 */
BlendFlag RenderState::getBlend() const {
	return pimpl->m_blend;
}

/*
 *
 */
const Lighting & RenderState::getLighting() const {
	return *pimpl->m_lighting;
}

/*
 *
 */
unsigned RenderState::getShaderId() const {
	return pimpl->m_shader->getId();
}

/*
 *
 */
std::unordered_set<int> RenderState::getTextureDependencies() const {
	return pimpl->m_uniforms->getTextureDependencies(pimpl->m_shader);
}

/*
 *
 */
Transform RenderState::getTransform() const {
	return *pimpl->m_transform;
}

/*
 *
 */
Vec3 RenderState::getTranslation() const {
	return pimpl->m_transform->getTranslation();
}

/**
 * test if state has given shader flag
 *
 * @return  true if state has shader flag, false otherwise
 */
bool RenderState::hasShaderFlag(size_t flag) const {
	return pimpl->m_tag->hasFlag(flag);
}

/**
 * test if state has given uniform
 *
 * @return  true if state has uniform, false otherwise
 */
bool RenderState::hasUniform(size_t uid) const {
	return pimpl->m_uniforms->contains(uid);
}

/*
 *
 */
bool RenderState::isInstanceable(const RenderState & other) const {
	if (pimpl->m_shader != other.pimpl->m_shader) {
		return false;
	}
	if (pimpl->m_blend != other.pimpl->m_blend) {
		return false;
	}
	if (pimpl->m_vertexAttributes->isInstanceable(
			*other.pimpl->m_vertexAttributes) == false) {
		return false;
	}
	if (*pimpl->m_uniforms != *other.pimpl->m_uniforms) {
		return false;
	}
	return true;
}

/*
 *
 */
void RenderState::overlayShaderTag(const ShaderTag & overlay) {
	makeUnique(pimpl);
	makeUnique(pimpl->m_tag);

	pimpl->m_tag->overlay(overlay);
}

/*
 *
 */
void RenderState::rotate(const Quat & rotation) {
	makeUnique(pimpl);
	makeUnique(pimpl->m_transform);

	pimpl->m_transform->rotate(rotation);
}

/*
 *
 */
void RenderState::setBlend(const BlendFlag & blend) {
	makeUnique(pimpl);

	pimpl->m_blend = blend;
}

/*
 *
 */
void RenderState::setSkinningMatrix(int index, const Transform & transform) {
	makeUnique(pimpl);
	makeUnique2(pimpl->m_skinningMatrices);

	(*pimpl->m_skinningMatrices)[index] = transform.asMat4();
}

/*
 *
 */
void RenderState::setupSkinning() {
	if (pimpl->m_skinningMatrices == nullptr) {
		return;
	}

	for (size_t i = 0, n = pimpl->m_skinningMatrices->size(); i < n; ++i) {
		auto name = "SkinningMatrices[" + std::to_string(i) + "]";
		addUniform(
				Uniform(Uniform::getUID(name),
						(*pimpl->m_skinningMatrices)[i]));
	}
}

/*
 *
 */
void RenderState::transform(const Transform & t) {
	makeUnique(pimpl);
	makeUnique(pimpl->m_transform);

	pimpl->m_transform->transform(t);
}

/*
 *
 */
void RenderState::translate(const Vec3 & translation) {
	makeUnique(pimpl);
	makeUnique(pimpl->m_transform);

	pimpl->m_transform->translate(translation);
}

/*
 *
 */
void RenderState::unbindShader() const {
	pimpl->m_shader->unbind();
}

/*
 *
 */
bool RenderState::validate() {
	if (pimpl->m_uniforms == nullptr || pimpl->m_vertexAttributes == nullptr) {
		return false;
	}

	// cleanse flags
	if (pimpl->m_tag->hasFlag(lightMapFlag)) {
		if (pimpl->m_uniforms->contains(LightMapUID) == false) {
			pimpl->m_tag->removeFlag(lightMapFlag);
		} else {
			const auto & u = pimpl->m_uniforms->get(LightMapUID);
			assert(u.getTexture() != nullptr);
			if (u.getTexture()->validate() == false) {
				pimpl->m_tag->removeFlag(lightMapFlag);
			}
		}
	}
	if (pimpl->m_tag->hasFlag(lumaMapFlag)) {
		if (pimpl->m_uniforms->contains(LumaMapUID) == false) {
			pimpl->m_tag->removeFlag(lumaMapFlag);
		} else {
			const auto & u = pimpl->m_uniforms->get(LumaMapUID);
			assert(u.getTexture() != nullptr);
			if (u.getTexture()->validate() == false) {
				pimpl->m_tag->removeFlag(lumaMapFlag);
			}
		}
	}

	// get shader
	pimpl->m_shader = ShaderManager::getInstance().getShader(*pimpl->m_tag);
	if (pimpl->m_shader == nullptr) {
		return false;
	}
	if (pimpl->m_vertexAttributes->validate() == false) {
		return false;
	}
	return true;
}
