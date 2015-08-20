#include "textureManager.h"

#include "imageFile.h"

#include "../core/config.h"
#include "../core/loadManager.h"
#include "../core/nameToIdMap.h"

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include <cassert>

#include <GL/glew.h>

using namespace render;

namespace {
	struct InternalCache {
		struct RuntimeTexture {
			std::shared_ptr<Texture> texture;
			size_t lastUsed;

			RuntimeTexture(const std::shared_ptr<Texture> & texture,
					size_t lastUsed) :
					texture(texture), lastUsed(lastUsed) {
			}
		};

		std::unordered_map<size_t, RuntimeTexture> cache;
		size_t counter = 0;

		std::shared_ptr<Texture> get(size_t id) {
			auto it = cache.find(id);
			if (it == cache.end()) {
				return nullptr;
			}
			it->second.lastUsed = counter;

			return it->second.texture;
		}

		void freshen(int id) {
			auto it = cache.find(id);
			assert(it != cache.end());
			it->second.lastUsed = std::max(it->second.lastUsed, counter);
		}

		void put(size_t id, const std::shared_ptr<Texture> & texture) {
			auto result = cache.emplace(id, RuntimeTexture(texture, counter));
			if (result.second == false) {
				result.first->second = RuntimeTexture(texture, counter);
			}
		}

		void scrub() {
			for (const auto & entry : cache) {
				if (entry.second.lastUsed >= counter) {
					continue;
				}

				entry.second.texture->invalidate();
			}
			counter++;
		}

		void clear() {
			for (const auto & entry : cache) {
				entry.second.texture->invalidate();
			}
			cache.clear();
			counter = 0;
		}
	};
}

struct TextureManager::impl {
	NameToIdMap nameToId;

	InternalCache internalCache;
	std::unordered_map<size_t, std::shared_ptr<Texture> > externalCache;

	std::unordered_map<size_t, std::vector<std::shared_ptr<Texture>>>cubeSet;

	std::shared_ptr<Texture> screen;

	impl() : screen(std::make_shared<Texture>(nameToId.getId("screen placeholder"), 0, 0, 0,
					GL_RGBA8, 0, 0,
					0, 0, 0)) {
	}
};

/**
 *
 */
TextureManager::TextureManager() :
		pimpl(new impl()) {
}

void TextureManager::freshen(const std::unordered_set<int> & textures) {
	for (const auto & id : textures) {
		auto it = pimpl->externalCache.find(id);
		if (it != pimpl->externalCache.end()) {
			continue;
		}

		pimpl->internalCache.freshen(id);
	}
}

/**
 * get 3D texture
 *
 * @param name    texture name
 * @param width   texture width
 * @param height  texture height
 * @param depth   texture depth
 * @param flags   texture flags
 *
 * @return        3d texture
 */
std::shared_ptr<Texture> TextureManager::get3dTexture(const std::string & name,
		unsigned width, unsigned height, unsigned depth,
		const FlagSet & flags) {
	auto id = getId(name);

	auto texture = pimpl->internalCache.get(id);

	if (texture != nullptr) {
		assert(texture->getWidth() == width);
		assert(texture->getHeight() == height);
		assert(texture->getDepth() == depth);
		assert(
				Config::getInstance().getBoolean("floatTextures") == false
						|| texture->isFloatingPoint());
		return texture;
	}

	int internalFormat;
	if (flags[Flag::RGBA8]) {
		internalFormat = GL_RGBA8;
	} else {
		assert(flags[Flag::RGBA16F]);
		internalFormat = GL_RGBA16F;
	}
	int filter = flags[Flag::FILTER] ? GL_LINEAR : GL_NEAREST;

	texture = std::make_shared<Texture>(id, width, height, depth,
			internalFormat, filter, filter, GL_CLAMP_TO_EDGE);

	pimpl->internalCache.put(id, texture);

	return texture;
}

/**
 * get cube map
 *
 * @param name    texture name
 * @param width   face width
 * @param height  face height
 * @param face    face index
 *                0 all faces,
 *                1 positive x,
 *                2 negative x,
 *                3 positive y,
 *                4 negative y,
 *                5 positive z,
 *                6 negative z
 * @return        cube map texture
 */
const std::shared_ptr<Texture> & TextureManager::getCubeMap(
		const std::string & name, unsigned width, unsigned height,
		Texture::Face face) {
	auto id = getId(name);

	auto texture = pimpl->internalCache.get(id);

	if (texture != nullptr) {
		assert(texture->getWidth() == width);
		assert(texture->getHeight() == height);
		assert(texture->isCubeMap());
		return pimpl->cubeSet.at(id)[static_cast<int>(face)];
	}

	texture = std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
			GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
			GL_TEXTURE_CUBE_MAP);

	pimpl->internalCache.put(id, texture);

	std::vector<std::shared_ptr<Texture>> faces = {
			texture,
			std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
					GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X),
			std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
					GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
					GL_TEXTURE_CUBE_MAP_NEGATIVE_X),
			std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
					GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
					GL_TEXTURE_CUBE_MAP_POSITIVE_Y),
			std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
					GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
					GL_TEXTURE_CUBE_MAP_NEGATIVE_Y),
			std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
					GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
					GL_TEXTURE_CUBE_MAP_POSITIVE_Z),
			std::make_shared<Texture>(id, width, height, GL_TEXTURE_CUBE_MAP,
					GL_RGBA8, GL_NONE, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE,
					GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) };

	pimpl->cubeSet.emplace(id, faces);

	return pimpl->cubeSet.at(id)[static_cast<int>(face)];
}

/*
 *
 */
const std::vector<std::shared_ptr<Texture>> & TextureManager::getCubeSet(
		int id) {
	return pimpl->cubeSet.at(id);
}

/*
 *
 */
std::shared_ptr<Texture> TextureManager::getDepthTexture(
		const std::string & name, unsigned width, unsigned height,
		const std::string & type) {
	auto id = getId(name);

	auto tex = pimpl->internalCache.get(id);

	const auto & depthFormat = Config::getInstance().getString("depthFormat");
	if (depthFormat == "RGBA_DEPTH") {
		if (tex != nullptr) {
			assert(tex->getWidth() == width);
			assert(tex->getHeight() == height);
			assert(tex->isFakeDepth());
			return tex;
		}
		// point sample
		tex = std::make_shared<Texture>(id, width, height, GL_TEXTURE_2D,
				GL_RGBA, GL_NONE, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE,
				GL_TEXTURE_2D);
		tex->setFakeDepth();
		pimpl->internalCache.put(id, tex);
		return pimpl->internalCache.get(id);
	}

	if (tex != nullptr) {
		assert(tex->getWidth() == width);
		assert(tex->getHeight() == height);
		assert(tex->isDepth());

		if (type == "lequal") {
			assert(tex->getCompareMode() == GL_COMPARE_R_TO_TEXTURE);
		} else {
			assert(type == "value");
			assert(tex->getCompareMode() == GL_NONE);
		}
		return tex;
	}

	if (type == "lequal") {
		// compare z-value bilinear
		tex = std::make_shared<Texture>(id, width, height, GL_TEXTURE_2D,
				GL_DEPTH_COMPONENT, GL_COMPARE_R_TO_TEXTURE, GL_LINEAR,
				GL_LINEAR, GL_CLAMP_TO_EDGE, GL_TEXTURE_2D);
		pimpl->internalCache.put(id, tex);
		return tex;
	}

	assert(type == "value");
	// point sampled z-value
	tex = std::make_shared<Texture>(id, width, height, GL_TEXTURE_2D,
			GL_DEPTH_COMPONENT, GL_NONE, GL_NEAREST, GL_NEAREST,
			GL_CLAMP_TO_EDGE, GL_TEXTURE_2D);
	pimpl->internalCache.put(id, tex);
	return tex;
}

/*
 *
 */
size_t TextureManager::getId(const std::string & name) {
	return pimpl->nameToId.getId(name);
}

/*
 *
 */
const std::shared_ptr<Texture> & TextureManager::getImage(
		const std::string & currentDir, const std::string & name,
		bool genmipmap, bool filter, bool wrap) {
	auto canonicalFlename = LoadManager::getInstance()->getName(currentDir,
			name);
	auto id = getId(canonicalFlename);

	auto it = pimpl->externalCache.find(id);
	if (it != pimpl->externalCache.end()) {
		return it->second;
	}

	auto file = std::unique_ptr<ImageFile>(new ImageFile(currentDir, name));

	auto tex = std::make_shared<Texture>(id, file, genmipmap, filter, wrap);
	pimpl->externalCache.emplace(id, tex);

	return pimpl->externalCache[id];
}

/*
 *
 */
const std::string & TextureManager::getName(int tid) {
	return pimpl->nameToId.getName(tid);
}

/**
 * get screen placeholder
 *
 * @return  placeholder texture for screen
 */
std::shared_ptr<Texture> TextureManager::getScreen() {
	return pimpl->screen;
}

/*
 *
 */
std::shared_ptr<Texture> TextureManager::getTexture(const std::string & name,
		unsigned width, unsigned height, bool filter) {
	auto id = getId(name);

	auto tex = pimpl->internalCache.get(id);

	if (tex != nullptr) {
		assert(tex->getWidth() == width);
		assert(tex->getHeight() == height);
		assert(tex->isDepth() == false && tex->isFakeDepth() == false);
		assert(tex->isCubeMap() == false);
		return tex;
	}

	int minFilter, magFilter;
	if (filter) {
		minFilter = GL_LINEAR;
		magFilter = GL_LINEAR;
	} else {
		minFilter = GL_NEAREST;
		magFilter = GL_NEAREST;
	}
	tex = std::make_shared<Texture>(id, width, height, GL_TEXTURE_2D, GL_RGBA8,
			GL_NONE, minFilter, magFilter, GL_CLAMP_TO_EDGE, GL_TEXTURE_2D);
	pimpl->internalCache.put(id, tex);

	return tex;
}

/*
 *
 */
STATIC TextureManager & TextureManager::getInstance() {
	static TextureManager instance;
	return instance;
}

/**
 * set width & height for screen placeholder
 *
 * @param width   screen width
 * @param height  screen height
 */
void TextureManager::setScreen(unsigned width, unsigned height) {
	pimpl->screen->resize(width, height);
}
