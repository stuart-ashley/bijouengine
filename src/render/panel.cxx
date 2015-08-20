#include "panel.h"

#include "canvas.h"
#include "glDebug.h"
#include "shader.h"
#include "texture.h"
#include "uniform.h"
#include "uniformArray.h"

#include "../core/color.h"
#include "../core/mat4.h"

#include <cmath>

#include <GL/glew.h>

using namespace render;

namespace {
	auto SrcDimensionsUID = Uniform::getUID("SrcDimensions");
	auto DestDimensionsUID = Uniform::getUID("DestDimensions");
	auto mvp_matrixUID = Uniform::getUID("mvp_matrix");
	auto colorUID = Uniform::getUID("color");
}

struct Panel::impl {
	Rect srcRect;
	UniformArray uniforms;
	std::shared_ptr<Shader> shader;
	std::shared_ptr<Texture> source;
	BlendFlag blend;
	Color color;
	float alpha;
	int srcWidth;
	int srcHeight;
	int dstWidth;
	int dstHeight;
	bool executed;

	impl(const Rect & srcRect, const UniformArray & uniforms,
			const std::shared_ptr<Shader> & shader,
			const std::shared_ptr<Texture> & source, BlendFlag blend,
			const Color & color, float alpha) :
					srcRect(srcRect),
					uniforms(uniforms),
					shader(shader),
					source(source),
					blend(blend),
					color(color),
					alpha(alpha),
					srcWidth(0),
					srcHeight(0),
					dstWidth(0),
					dstHeight(0),
					executed(false) {
		assert(shader!=nullptr);
	}

	Mat4 calculateMatrix(const Rect & viewportRect) {
		// destination width, height & aspect ratio
		dstWidth = static_cast<int>(viewportRect.getWidth());
		dstHeight = static_cast<int>(viewportRect.getHeight());
		float dstAspect = std::abs(viewportRect.getAspectRatio());

		// source width & height
		float srcAspect = dstAspect;
		int srcw = dstWidth;
		int srch = dstHeight;
		if (source != nullptr) {
			if (source->validate()) {
				srcw = source->getWidth();
				srch = source->getHeight();
				srcAspect = static_cast<float>(srcw) / static_cast<float>(srch);
			}
		}
		srcWidth = srcw;
		srcHeight = srch;
		srcAspect *= srcRect.getAspectRatio();

		// matrix
		if (dstAspect > srcAspect) {
			float a = srcAspect / dstAspect;
			return Mat4(2 * a, 0, 0, -a, 0, 2, 0, -1, 0, 0, -1, 0, 0, 0, 0, 1);
		} else {
			float a = dstAspect / srcAspect;
			return Mat4(2, 0, 0, -1, 0, 2 * a, 0, -a, 0, 0, -1, 0, 0, 0, 0, 1);
		}
	}
};

/**
 *
 * @param name
 * @param srcRect
 * @param dstRect
 * @param uniforms
 * @param shader
 * @param texture
 * @param source
 * @param blend
 * @param level
 * @param color
 * @param alpha
 * @param opaque
 * @param bgColor
 */
Panel::Panel(const std::string & name, const Rect & srcRect,
		const Rect & dstRect, const UniformArray & uniforms,
		const std::shared_ptr<Shader> & shader,
		const std::shared_ptr<Texture> & texture,
		const std::shared_ptr<Texture> & source, BlendFlag blend, int level,
		const Color & color, float alpha, bool opaque, const Color & bgColor) :
				RenderTask(name, texture, dstRect,
						uniforms.getTextureDependencies(shader), level),
				pimpl(
						new impl(srcRect, uniforms, shader, source, blend,
								color, alpha)) {
	setOpaque(opaque);
	setBGColor(bgColor);
}

/**
 * destructor
 */
Panel::~Panel() {
}

/**
 *
 */
OVERRIDE void Panel::execute(Canvas & canvas) {
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
		const auto & bg = getBGColor();
		glClearColor(bg.getR(), bg.getG(), bg.getB(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	switch (pimpl->blend) {
	case BlendFlag::ADDITIVE:
		glEnable(GL_BLEND);
		glBlendColor(0, 0, 0, pimpl->alpha);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
		break;
	case BlendFlag::SRC_ALPHA_ADDITIVE:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case BlendFlag::SRC_ALPHA:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	default:
		break;
	}

	glEnable(GL_DEPTH_TEST); // update z-buffer
	glDepthFunc(GL_ALWAYS); // overlay

	pimpl->shader->bind();

	// add special uniforms, this is an update to 'uniforms' during
	// rendering thread and requires some synchronisation in
	// 'UniformArray'
	UniformArray uforms;
	uforms.add(Uniform(mvp_matrixUID, pimpl->calculateMatrix(rect)));
	uforms.add(
			Uniform(colorUID, pimpl->color.getR(), pimpl->color.getG(),
					pimpl->color.getB(), pimpl->alpha));
	uforms.add(
			Uniform(SrcDimensionsUID, static_cast<float>(pimpl->srcWidth),
					static_cast<float>(pimpl->srcHeight)));
	uforms.add(
			Uniform(DestDimensionsUID, static_cast<float>(width),
					static_cast<float>(height)));
	uforms.add(pimpl->uniforms);
	uforms.bind(pimpl->shader);

	// get attribute indices
	int posIdx = pimpl->shader->getAttributeIndex("a_position");
	int uvIdx = pimpl->shader->getAttributeIndex("a_uv");

	// setup quad
	float uv[] = { pimpl->srcRect.getLeft(), pimpl->srcRect.getBottom(),
			pimpl->srcRect.getLeft(), pimpl->srcRect.getTop(),
			pimpl->srcRect.getRight(), pimpl->srcRect.getTop(),
			pimpl->srcRect.getRight(), pimpl->srcRect.getBottom() };
	float xy[] = { 0, 0, 0, 1, 1, 1, 1, 0 };

	// setup attributes
	glVertexAttribPointer(posIdx, 2, GL_FLOAT, false, 0, xy);
	if (uvIdx >= 0) {
		glVertexAttribPointer(uvIdx, 2, GL_FLOAT, false, 0, uv);
	}

	// draw quad
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// disable attributes
	pimpl->shader->unbind();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	glFlush();
	GlDebug::printOpenGLError();

	pimpl->executed = true;
}

/**
 *
 * @return
 */
OVERRIDE bool Panel::hasExecuted() const {
	return pimpl->executed;
}
