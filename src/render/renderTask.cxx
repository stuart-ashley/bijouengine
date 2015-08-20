#include "renderTask.h"

#include "texture.h"

#include "../core/color.h"

using namespace render;

namespace {
	std::vector<std::shared_ptr<Texture>> textureAsList(
			const std::shared_ptr<Texture> & texture) {
		std::vector<std::shared_ptr<Texture>> list;
		list.emplace_back(texture);
		return list;
	}
}

struct RenderTask::impl {
	std::string name;
	std::vector<std::shared_ptr<Texture>> textures;
	std::unordered_set<int> textureDependencies;
	/** target rectangle */
	Rect rect;
	/** higher level, drawn later */
	int level;
	/** opaque flag */
	bool opaque;
	/** background color */
	Color bgColor;

	/*
	 *
	 */
	impl(const std::string & name, const std::shared_ptr<Texture> & texture,
			const Rect & rect, int level) :
					name(name),
					textures(textureAsList(texture)),
					rect(rect),
					level(level),
					opaque(true),
					bgColor(Color::black()) {
	}

	/*
	 *
	 */
	impl(const std::string & name,
			const std::vector<std::shared_ptr<Texture>> & textures,
			const Rect & rect, int level) :
					name(name),
					textures(textures),
					rect(rect),
					level(level),
					opaque(true),
					bgColor(Color::black()) {
	}

	/*
	 *
	 */
	impl(const std::string & name, const std::shared_ptr<Texture> & texture,
			const Rect & rect, const std::unordered_set<int> & dependencies,
			int level) :
					name(name),
					textures(textureAsList(texture)),
					textureDependencies(dependencies),
					rect(rect),
					level(level),
					opaque(true),
					bgColor(Color::black()) {
	}
};

/**
 * destructor
 */
VIRTUAL RenderTask::~RenderTask() {
}

/*
 *
 */
void RenderTask::addDependencies(const std::unordered_set<int> & textureIds) {
	pimpl->textureDependencies.insert(textureIds.begin(), textureIds.end());
}

/*
 *
 */
void RenderTask::addDependency(int textureId) {
	pimpl->textureDependencies.emplace(textureId);
}

/*
 *
 */
const Color & RenderTask::getBGColor() const {
	return pimpl->bgColor;
}

/*
 *
 */
int RenderTask::getLevel() const {
	return pimpl->level;
}

/*
 *
 */
const std::string & RenderTask::getName() const {
	return pimpl->name;
}

/*
 *
 */
bool RenderTask::getOpaque() const {
	return pimpl->opaque;
}

/*
 *
 */
const Rect & RenderTask::getRect() const {
	return pimpl->rect;
}

/*
 *
 */
Rect RenderTask::getScissor() const {
	float w, h;
	const auto & target = pimpl->textures.at(0);
	w = static_cast<float>(target->getWidth());
	h = static_cast<float>(target->getHeight());

	return Rect(w * pimpl->rect.getLeft(), w * pimpl->rect.getRight(),
			h * pimpl->rect.getBottom(), h * pimpl->rect.getTop());
}

/*
 *
 */
const std::vector<std::shared_ptr<Texture>> & RenderTask::getTargetTextures() const {
	return pimpl->textures;
}

/*
 *
 */
const std::unordered_set<int> & RenderTask::getTextureDependencies() const {
	return pimpl->textureDependencies;
}

/*
 *
 */
Rect RenderTask::getViewport() const {
	float w, h;
	const auto & target = pimpl->textures.at(0);
	w = static_cast<float>(target->getWidth());
	h = static_cast<float>(target->getHeight());

	return Rect(w * pimpl->rect.getLeft(), w * pimpl->rect.getRight(),
			h * pimpl->rect.getBottom(), h * pimpl->rect.getTop());
}

/*
 *
 */
bool RenderTask::isCubeFace() const {
	const auto & target = pimpl->textures.at(0);
	if (target == nullptr) {
		return false;
	}
	return target->isCubeMap();
}

/*
 *
 */
void RenderTask::setBGColor(const Color & col) {
	pimpl->bgColor = col;
}

/*
 *
 */
void RenderTask::setOpaque(bool opaque) {
	pimpl->opaque = opaque;
}

/*
 *
 */
void RenderTask::setRect(const Rect & rect) {
	pimpl->rect = rect;
}

/*
 *
 */
PROTECTED RenderTask::RenderTask(const std::string & name,
		const std::shared_ptr<Texture> & texture, const Rect & rect, int level) :
		pimpl(new impl(name, texture, rect, level)) {
}

/*
 *
 */
PROTECTED RenderTask::RenderTask(const std::string & name,
		const std::vector<std::shared_ptr<Texture>> & textures,
		const Rect & rect, int level) :
		pimpl(new impl(name, textures, rect, level)) {
}

/*
 *
 */
PROTECTED RenderTask::RenderTask(const std::string & name,
		const std::shared_ptr<Texture> & textures, const Rect & rect,
		const std::unordered_set<int> & dependencies, int level) :
		pimpl(new impl(name, textures, rect, dependencies, level)) {
}
