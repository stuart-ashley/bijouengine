#pragma once

#include "blendFlag.h"
#include "shaderFlag.h"

#include <memory>
#include <string>
#include <unordered_set>

class BoundingBox;
class Quat;
class Transform;
class Vec3;

namespace render {
	class Lighting;
	class Shader;
	class ShaderTag;
	class Uniform;
	class UniformArray;
	class VertexAttribute;

	class RenderState {
	public:

		inline explicit RenderState(const RenderState &) = default;

		inline ~RenderState() = default;

		inline RenderState & operator=(const RenderState &) = default;

		RenderState(const Lighting & lighting, const ShaderTag & tag);

		void addShaderFlag(size_t flag);

		void addShaderFlags(const ShaderFlags & flags);

		void addUniform(const Uniform & uniform);

		void addUniforms(const UniformArray & uniforms);

		void addVertexAttribute(const VertexAttribute & attribute);

		void bindShader() const;

		void bindUniforms() const;

		void bindVertexAttributes() const;

		void clipLights(const BoundingBox & bounds);

		BlendFlag getBlend() const;

		const Lighting & getLighting() const;

		unsigned getShaderId() const;

		std::unordered_set<int> getTextureDependencies() const;

		Transform getTransform() const;

		Vec3 getTranslation() const;

		/**
		 * test if state has given shader flag
		 *
		 * @return  true if state has shader flag, false otherwise
		 */
		bool hasShaderFlag(size_t flag) const;

		/**
		 * test if state has given uniform
		 *
		 * @return  true if state has uniform, false otherwise
		 */
		bool hasUniform(size_t uid) const;

		bool isInstanceable(const RenderState & other) const;

		void overlayShaderTag(const ShaderTag & overlay);

		void rotate(const Quat & rotation);

		void setBlend(const BlendFlag & blend);

		void setSkinningMatrix(int index, const Transform & transform);

		void setupSkinning();

		void transform(const Transform & t);

		void translate(const Vec3 & translation);

		void unbindShader() const;

		bool validate();

	private:
		struct impl;
		std::shared_ptr<impl> pimpl;
	};
}

