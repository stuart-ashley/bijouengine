#include "texture.h"

#include "glDebug.h"
#include "imageFile.h"
#include "textureManager.h"

#include "../scripting/bool.h"
#include "../scripting/executable.h"
#include "../scripting/kwarg.h"
#include "../scripting/parameters.h"
#include "../scripting/real.h"
#include "../scripting/string.h"
#include "../scripting/scriptExecutionException.h"

#include <cassert>
#include <iostream>
#include <mutex>

#include <GL/glew.h>

using namespace render;

namespace {

	std::vector<BaseParameter> imageParams = {
			Parameter<String>("file", nullptr),
			Parameter<Bool>("genmipmap", Bool::True()),
			Parameter<Bool>("filter", Bool::True()) };

	std::vector<BaseParameter> targetParams = {
			Parameter<String>("name", nullptr),
			Parameter<Real>("width", nullptr),
			Parameter<Real>("height", nullptr),
			Parameter<Bool>("cube", Bool::False()),
			Parameter<Bool>("filter", Bool::True()),
			Parameter<String>("shadow", std::make_shared<String>("")) };

	struct Factory: public Executable {
		std::string currentDir;
		Parameters imageParameters;
		Parameters targetParameters;

		Factory(const std::string & currentDir) :
						currentDir(currentDir),
						imageParameters(imageParams),
						targetParameters(targetParams) {
		}

		void doImage(unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const {
			auto args = imageParameters.getArgs(nArgs, stack);

			auto file =
					std::static_pointer_cast<String>(args["file"])->getValue();

			auto genmipmap = std::static_pointer_cast<Bool>(args["genmipmap"])
					== Bool::True();

			auto filter = std::static_pointer_cast<Bool>(args["filter"])
					== Bool::True();

			stack.emplace(
					TextureManager::getInstance().getImage(currentDir, file,
							genmipmap, filter, true));

		}

		void doTarget(unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const {
			auto args = targetParameters.getArgs(nArgs, stack);

			auto name =
					std::static_pointer_cast<String>(args["name"])->getValue();

			auto width =
					std::static_pointer_cast<Real>(args["width"])->getInt32();

			auto height =
					std::static_pointer_cast<Real>(args["height"])->getInt32();

			auto cube = std::static_pointer_cast<Bool>(args["cube"])
					== Bool::True();

			auto filter = std::static_pointer_cast<Bool>(args["filter"])
					== Bool::True();

			auto shadow =
					std::static_pointer_cast<String>(args["shadow"])->getValue();

			if (shadow != "") {
				stack.emplace(
						TextureManager::getInstance().getDepthTexture(name,
								width, height, shadow));
			} else {
				if (cube) {
					stack.emplace(
							TextureManager::getInstance().getCubeMap(name,
									width, height, Texture::Face::NONE));
				} else {
					stack.emplace(
							TextureManager::getInstance().getTexture(name,
									width, height, filter));
				}
			}

		}

		void execute(const ScriptObjectPtr &, unsigned nArgs,
				std::stack<ScriptObjectPtr> & stack) const override {
			auto arg = stack.top();

			if (typeid(*arg) == typeid(Kwarg)) {
				// keyword name
				const auto & name =
						(std::static_pointer_cast<Kwarg>(arg))->getKey();

				if (name == "file") {
					doImage(nArgs, stack);
				} else if (name == "name") {
					doTarget(nArgs, stack);
				} else {
					scriptExecutionAssert(false,
							"Require file or name as first keyword");
				}
			} else {
				scriptExecutionAssert(false, "Require keyword");
			}
		}
	};
}

struct Texture::impl {
	/** lock for synchronization */
	std::mutex lock;
	/** whether texture has been generated ( generation deferred until bind ) */
	bool generated;
	/** texture manager id */
	int tid;
	/** texture width */
	unsigned width;
	/** texture height */
	unsigned height;
	/** texture depth */
	unsigned depth;
	/** OpenGL texture id */
	GLuint id;
	/** type GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP */
	int type;
	/** format GL_RGB8, GL_DEPTH_COMPONENT or GL_RGB32F */
	int internalFormat;
	/** depth compare mode GL_COMPARE_R_TO_TEXTURE or GL_NONE */
	int compareMode;
	/** shrink filter GL_LINEAR or GL_NEAREST */
	int minFilter;
	/** expand filter GL_LINEAR or GL_NEAREST */
	int magFilter;
	/** tiling mode GL_CLAMP_TO_EDGE or GL_REPEAT */
	int wrap;
	/**
	 * GL texture target, GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	 * GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	 * GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	 * GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	 */
	int target;
	/** image */
	std::unique_ptr<ImageFile> image;
	bool fakeDepth;

	impl(int tid, unsigned width, unsigned height, int type, int internalFormat,
			int compareMode, int minFilter, int magFilter, int wrap, int target) :
					generated(false),
					tid(tid),
					width(width),
					height(height),
					depth(1),
					id(0),
					type(type),
					internalFormat(internalFormat),
					compareMode(compareMode),
					minFilter(minFilter),
					magFilter(magFilter),
					wrap(wrap),
					target(target),
					fakeDepth(false) {
	}

	impl(int tid, unsigned width, unsigned height, unsigned depth,
			int internalFormat, int minFilter, int magFilter, int wrap) :
					generated(false),
					tid(tid),
					width(width),
					height(height),
					depth(depth),
					id(0),
					type(GL_TEXTURE_3D),
					internalFormat(internalFormat),
					compareMode(GL_NONE),
					minFilter(minFilter),
					magFilter(magFilter),
					wrap(wrap),
					target(GL_TEXTURE_3D),
					fakeDepth(false) {
	}

	impl(int tid, std::unique_ptr<ImageFile> & image, bool genmipmap,
			bool filter, bool wrap) :
					generated(false),
					tid(tid),
					width(0),
					height(0),
					depth(0),
					id(0),
					type(GL_TEXTURE_2D),
					internalFormat(0),
					compareMode(GL_NONE),
					minFilter(
							genmipmap ?
									(filter ?
											GL_LINEAR_MIPMAP_LINEAR :
											GL_NEAREST_MIPMAP_NEAREST) :
									(filter ? GL_LINEAR : GL_NEAREST)),
					magFilter(filter ? GL_LINEAR : GL_NEAREST),
					wrap(wrap ? GL_REPEAT : GL_CLAMP),
					target(GL_TEXTURE_2D),
					image(std::move(image)),
					fakeDepth(false) {
	}
};

/**
 * construct 2d texture
 *
 * @param tid             unique texture id
 * @param width           texture width
 * @param height          texture height
 * @param type            texture type ( regular or cube map )
 * @param internalFormat  format ( byte, depth or float )
 * @param compareMode     depth compare mode
 * @param minFilter       shrink filter
 * @param magFilter       expand filter
 * @param wrap            tiling mode
 * @param target          texture target ( regular or face of cube map )
 */
Texture::Texture(int tid, unsigned width, unsigned height, int type,
		int internalFormat, int compareMode, int minFilter, int magFilter,
		int wrap, int target) :
				pimpl(
						new impl(tid, width, height, type, internalFormat,
								compareMode, minFilter, magFilter, wrap,
								target)) {
}

/**
 * construct 3d texture
 *
 * @param tid             unique texture id
 * @param width           texture width
 * @param height          texture height
 * @param depth           texture depth
 * @param internalFormat  internal format GL_RGBA8 or GL_RGBA16F
 * @param minFilter       miniaturise filter GL_LINEAR or GL_NEAREST
 * @param magFilter       magnify filter GL_LINEAR or GL_NEAREST
 * @param wrap            tiling mode GL_CLAMP_TO_EDGE
 */
Texture::Texture(int tid, unsigned width, unsigned height, unsigned depth,
		int internalFormat, int minFilter, int magFilter, int wrap) :
				pimpl(
						new impl(tid, width, height, depth, internalFormat,
								minFilter, magFilter, wrap)) {
}

/**
 * Construct texture from an image
 *
 * @param tid        unique texture id
 * @param image      image to use
 * @param genmipmap  should a mipmap be generated
 * @param filter     miniaturise &  magnify filter
 * @param wrap       tiling mode
 */
Texture::Texture(int tid, std::unique_ptr<ImageFile> & image, bool genmipmap,
		bool filter, bool wrap) :
		pimpl(new impl(tid, image, genmipmap, filter, wrap)) {
}

/*
 *
 */
Texture::~Texture() {
}

/**
 * Bind texture, if this is the first bind the texture needs to be setup
 *
 * unsynchronized, only previously validated textures should get this far
 */
void Texture::bind() {
	if (pimpl->generated) {
		glBindTexture(pimpl->type, pimpl->id);
		GlDebug::printOpenGLError();
		return;
	}

	if (pimpl->image != nullptr) {
		// after validate image is empty, pixels full
		if (validate() == false) {
			std::cerr << "Can't validate texture" << std::endl;
			assert(false);
		}
	}

	glGenTextures(1, &pimpl->id);
	glBindTexture(pimpl->type, pimpl->id);
	pimpl->generated = true;

	if (pimpl->type == GL_TEXTURE_3D) {
		glTexParameteri(pimpl->type, GL_TEXTURE_MIN_FILTER, pimpl->minFilter);
		glTexParameteri(pimpl->type, GL_TEXTURE_MAG_FILTER, pimpl->magFilter);
		glTexParameteri(pimpl->type, GL_TEXTURE_WRAP_S, pimpl->wrap);
		glTexParameteri(pimpl->type, GL_TEXTURE_WRAP_T, pimpl->wrap);
		glTexParameteri(pimpl->type, GL_TEXTURE_WRAP_R, pimpl->wrap);
		glTexImage3D(pimpl->type, 0, GL_RGBA16F, pimpl->width, pimpl->height,
				pimpl->depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		GlDebug::printOpenGLError();
		return;
	}

	int externalFormat = GL_RGBA;

	if (isDepth() == true) {
		assert(pimpl->type == GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
				pimpl->compareMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		externalFormat = GL_DEPTH_COMPONENT;
	}

	glTexParameteri(pimpl->type, GL_TEXTURE_MIN_FILTER, pimpl->minFilter);
	glTexParameteri(pimpl->type, GL_TEXTURE_MAG_FILTER, pimpl->magFilter);
	glTexParameteri(pimpl->type, GL_TEXTURE_WRAP_S, pimpl->wrap);
	glTexParameteri(pimpl->type, GL_TEXTURE_WRAP_T, pimpl->wrap);

	if (isCubeMap()) {
		assert(pimpl->image == nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, pimpl->width,
				pimpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, pimpl->width,
				pimpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, pimpl->width,
				pimpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, pimpl->width,
				pimpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, pimpl->width,
				pimpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, pimpl->width,
				pimpl->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		for (const auto & tex : TextureManager::getInstance().getCubeSet(
				pimpl->tid)) {
			tex->pimpl->generated = true;
			tex->pimpl->id = pimpl->id;
		}

		GlDebug::printOpenGLError();
		return;
	}

	assert(pimpl->type == GL_TEXTURE_2D);

	if (pimpl->image != nullptr) {
		// hand over custody of pixels to OpenGL
		auto externalFormat = pimpl->image->getPixelFormat();
		auto data = pimpl->image->getPixelData();

		if (pimpl->minFilter == GL_LINEAR_MIPMAP_LINEAR) {
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, pimpl->internalFormat, pimpl->width,
				pimpl->height, 0, externalFormat, GL_UNSIGNED_BYTE, data);

		// image data now in the hands of OpenGL
		pimpl->image = nullptr;

		GlDebug::printOpenGLError();

		return;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, pimpl->internalFormat, pimpl->width,
			pimpl->height, 0, externalFormat, GL_UNSIGNED_BYTE, nullptr);
	GlDebug::printOpenGLError();
}

/*
 *
 */
int Texture::getCompareMode() const {
	return pimpl->compareMode;
}

/*
 *
 */
unsigned Texture::getDepth() const {
	return pimpl->depth;
}

/*
 *
 */
unsigned Texture::getHeight() const {
	return pimpl->height;
}

/*
 *
 */
int Texture::getId() const {
	return pimpl->tid;
}

/**
 * get texture internal format
 *
 * @return  texture internal format
 */
int Texture::getInternalFormat() const {
	return pimpl->internalFormat;
}

/*
 *
 */
int Texture::getType() const {
	return pimpl->type;
}

/*
 *
 */
unsigned Texture::getWidth() const {
	return pimpl->width;
}

/*
 *
 */
int Texture::glId() const {
	return pimpl->id;
}

/*
 *
 */
int Texture::glTarget() const {
	return pimpl->target;
}

/**
 * free memory used by texture
 *
 * unsyncronized, to be called from draw thread
 */
void Texture::invalidate() {
	if (pimpl->generated) {
		glDeleteTextures(1, &pimpl->id);
		pimpl->generated = false;
		pimpl->id = 0;
		if (isCubeMap()) {
			for (const auto & tex : TextureManager::getInstance().getCubeSet(
					pimpl->tid)) {
				tex->pimpl->generated = false;
				tex->pimpl->id = 0;
			}
		}
	}
}

/*
 *
 */
bool Texture::is3d() const {
	return pimpl->type == GL_TEXTURE_3D;
}

/*
 *
 */
bool Texture::isCubeMap() const {
	return pimpl->type == GL_TEXTURE_CUBE_MAP;
}

/*
 *
 */
bool Texture::isDepth() const {
	return pimpl->internalFormat == GL_DEPTH_COMPONENT;
}

/**
 * is texture current, (unused textures are deleted from opengl context)
 *
 * @return  true if texture current, false otherwise
 */
bool Texture::isEnabled() const {
	return pimpl->generated;
}

/*
 *
 */
bool Texture::isFakeDepth() const {
	return pimpl->fakeDepth;
}

/*
 *
 */
bool Texture::isFloatingPoint() const {
	return pimpl->internalFormat == GL_RGB32F
			|| pimpl->internalFormat == GL_RGBA16F;
}

/**
 * declare this texture a fake depth texture
 *
 * unsynchronized, expected to be called at texture construction
 */
void Texture::setFakeDepth() {
	assert(pimpl->generated == false);
	pimpl->fakeDepth = true;
}

/**
 * resize texture
 *
 * unsynchronized, needs to be called in draw thread
 *
 * @param width   new texture width
 * @param height  new texture height
 */
void Texture::resize(unsigned width, unsigned height) {
	assert(pimpl->generated == false);
	pimpl->width = width;
	pimpl->height = height;
}

/**
 * validate texture, if an image check its loaded before use
 *
 * synchronized, multiple render graph generation threads may be calling this
 */
bool Texture::validate() {
	std::lock_guard<std::mutex> locker(pimpl->lock);

	if (pimpl->image == nullptr) {
		return true;
	}
	if (pimpl->image->valid()) {
		if (pimpl->internalFormat == 0) {
			pimpl->width = pimpl->image->getPixelWidth();
			pimpl->height = pimpl->image->getPixelHeight();
			if (pimpl->image->isLuminance()) {
				pimpl->internalFormat = GL_LUMINANCE;
			} else {
				pimpl->internalFormat = GL_RGBA8;
			}
		}
		return true;
	}
	return false;
}

/**
 * get script object factory for Texture
 *
 * @param currentDir  current directory of caller
 *
 * @return            Texture factory
 */
STATIC ScriptObjectPtr Texture::getFactory(const std::string & currentDir) {
	return std::static_pointer_cast<ScriptObject>(
			std::make_shared<Factory>(currentDir));
}

