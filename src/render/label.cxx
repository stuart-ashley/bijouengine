#include "label.h"

#include "canvas.h"
#include "decal.h"
#include "font.h"
#include "glDebug.h"
#include "shaderManager.h"
#include "texture.h"
#include "uniform.h"
#include "uniformArray.h"

#include "../core/color.h"
#include "../core/mat4.h"

#include <cmath>

#include <GL/glew.h>

using namespace render;

namespace {

	auto mvp_matrixUID = Uniform::getUID("mvp_matrix");
	auto colorUID = Uniform::getUID("color");
	auto sourceUID = Uniform::getUID("Source");

	Mat4 projectionMatrix(2, 0, 0, -1, 0, 2, 0, -1, 0, 0, -1, 0, 0, 0, 0, 1);

	struct Line {
		std::vector<std::shared_ptr<Decal>> decals;
		float width = 0;
		float height = 0;
	};
}

struct Label::impl {
	std::shared_ptr<Font> font;
	std::string justify;
	Color color;
	std::string text;
	float maxWidth = 0;
	float totalHeight = 0;
	std::vector<Line> lines;
	bool executed = false;

	impl(const std::shared_ptr<Font> & font, const std::string & justify,
			const Color & color, const std::string & text) :
			font(font), justify(justify), color(color), text(text) {
		if (font->validate()) {
			setText(text);
		}
	}

	void setText(const std::string & text2) {
		if (text2 == "") {
			lines.clear();
			return;
		}

		std::vector<Line> lines2;
		Line line;
		for (size_t i = 0, n = text2.length(); i < n; ++i) {
			char c = text2[i];
			if (c == '\n') {
				lines2.emplace_back(line);
				maxWidth = std::max(maxWidth, line.width);
				totalHeight += line.height;
				line = Line();
				continue;
			}
			const auto & decal = font->getDecal(c);
			line.decals.emplace_back(decal);

			const auto & dr = decal->getRect();

			line.height = std::max(line.height,
					std::abs(dr.getTop() - dr.getBottom()));
			line.width += dr.getWidth();
		}
		if (line.decals.size() > 0) {
			lines2.emplace_back(line);
			maxWidth = std::max(maxWidth, line.width);
			totalHeight += line.height;
		}
		lines = lines2;
	}
};

Label::Label(const std::string & name, const std::shared_ptr<Font> & font,
		const std::string & justify, const Rect & rect,
		const std::shared_ptr<Texture> & texture, const Color & color,
		bool opaque, const Color & bgColor, const std::string & text, int level) :
				RenderTask(name, texture, rect, level),
				pimpl(new impl(font, justify, color, text)) {
	setOpaque(opaque);
	setBGColor(bgColor);
}

/**
 * destructor
 */
Label::~Label() {
}

/**
 * execute label task
 */
void Label::execute(Canvas & canvas) {
	// check font loaded, set text if required
	if (pimpl->lines.size() == 0 && pimpl->text.length() > 0) {
		if (pimpl->font->validate()) {
			pimpl->setText(pimpl->text);
		} else {
			return;
		}
	}

	// set target
	if (canvas.setTarget(getTargetTextures()) == false) {
		return;
	}

	auto shader = ShaderManager::getInstance().getShader(
			pimpl->font->getShader());
	if (shader == nullptr) {
		return;
	}

	Rect rect = getViewport();
	int width = static_cast<int>(std::round(rect.getWidth()));
	int height = static_cast<int>(std::round(rect.getHeight()));
	glViewport(static_cast<GLint>(std::round(rect.getLeft())),
			static_cast<GLint>(std::round(rect.getBottom())), width, height);

	// scissor
	glEnable(GL_SCISSOR_TEST);
	Rect scissor = getScissor();
	glScissor(static_cast<GLint>(std::round(scissor.getLeft())),
			static_cast<GLint>(std::round(scissor.getBottom())),
			static_cast<GLsizei>(std::round(scissor.getWidth())),
			static_cast<GLsizei>(std::round(scissor.getHeight())));

	// clear
	if (getOpaque()) {
		const auto & bg = getBGColor();
		glClearColor(bg.getR(), bg.getG(), bg.getB(), 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// enable blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// assuming square target pixels
	float destAspect = static_cast<float>(width) / static_cast<float>(height);
	float textAspect = pimpl->maxWidth / pimpl->totalHeight;

	// current decal texture
	std::shared_ptr<Texture> decalTexture = nullptr;

	// bind shader
	shader->bind();

	// attribute indices
	int posIdx = shader->getAttributeIndex("a_position");
	int uvIdx = shader->getAttributeIndex("a_uv");
	assert(posIdx >= 0);
	assert(uvIdx >= 0);

	GlDebug::printOpenGLError();

	// calculate scale
	float sx, sy;
	if (destAspect < textAspect) {
		sx = 1 / pimpl->maxWidth;
		sy = destAspect / textAspect / pimpl->totalHeight;
	} else {
		sx = textAspect / destAspect / pimpl->maxWidth;
		sy = 1 / pimpl->totalHeight;
	}

	// centre vertically
	float tbottom = .5f + sy * pimpl->totalHeight / 2;

	for (Line line : pimpl->lines) {
		float tleft = 0.f;
		if (pimpl->justify == "centre") {
			tleft = .5f - sx * line.width / 2;
		}
		if (pimpl->justify == "right") {
			tleft = 1 - sx * line.width;
		}

		float ttop = tbottom;
		tbottom = ttop - sy * line.height;

		for (const auto & decal : line.decals) {
			float tright = tleft + sx * decal->getRect().getWidth();

			// new decal texture
			if (decalTexture != decal->getTexture()) {
				// validate decal texture
				if (decal->getTexture()->validate() == false) {
					continue;
				}

				// uniforms
				UniformArray uniforms;
				uniforms.add(Uniform(mvp_matrixUID, projectionMatrix));
				uniforms.add(Uniform(colorUID, pimpl->color));
				uniforms.add(Uniform(sourceUID, decal->getTexture()));
				uniforms.bind(shader);

				decalTexture = decal->getTexture();
			}

			const auto & drect = decal->getRect();
			float u0 = drect.getLeft();
			float u1 = drect.getRight();

			float v0, v1;
			if (decal->getFlip()) {
				v0 = drect.getTop();
				v1 = drect.getBottom();
			} else {
				v1 = drect.getTop();
				v0 = drect.getBottom();
			}

			float uv[] = { u0, v0, u0, v1, u1, v1, u1, v0 };

			float xy[] = { tleft, tbottom, tleft, ttop, tright, ttop, tright,
					tbottom };

			glVertexAttribPointer(posIdx, 2, GL_FLOAT, false, 0, xy);
			glVertexAttribPointer(uvIdx, 2, GL_FLOAT, false, 0, uv);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			GlDebug::printOpenGLError();

			// move forward
			tleft = tright;
		}
	}

	// disable attributes
	shader->unbind();

	GlDebug::printOpenGLError();
	glDisable(GL_BLEND);
	glDisable(GL_SCISSOR_TEST);

	pimpl->executed = true;
}

/*
 *
 */
bool Label::hasExecuted() const {
	return pimpl->executed;
}

/*
 *
 */
void Label::setText(const std::string & text) {
	pimpl->setText(text);
}
