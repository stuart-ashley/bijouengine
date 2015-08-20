#pragma once

#include "imageFile.h"

#include "../scripting/scriptObject.h"

namespace render {
	class Texture: public ScriptObject {
	public:
		enum class Face {
			NONE, POSITIVE_X, NEGATIVE_X, POSITIVE_Y, NEGATIVE_Y, POSITIVE_Z,
			NEGATIVE_Z
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
		Texture(int tid, unsigned width, unsigned height, int type,
				int internalFormat, int compareMode, int minFilter,
				int magFilter, int wrap, int target);

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
		Texture(int tid, unsigned width, unsigned height, unsigned depth,
				int internalFormat, int minFilter, int magFilter, int wrap);

		/**
		 * Construct texture from an image
		 *
		 * @param tid        unique texture id
		 * @param image      image to use
		 * @param genmipmap  should a mipmap be generated
		 * @param filter     miniaturise &  magnify filter
		 * @param wrap       tiling mode
		 */
		Texture(int tid, std::unique_ptr<ImageFile> & image, bool genmipmap,
				bool filter, bool wrap);

		~Texture();

		/**
		 * Bind texture, if this is the first bind the texture needs to be setup
		 *
		 * unsynchronized, only previously validated textures should get this far
		 */
		void bind();

		int getCompareMode() const;

		unsigned getDepth() const;

		unsigned getHeight() const;

		int getId() const;

		/**
		 * get texture internal format
		 *
		 * @return  texture internal format
		 */
		int getInternalFormat() const;

		int getType() const;

		unsigned getWidth() const;

		int glId() const;

		int glTarget() const;

		/**
		 * free memory used by texture
		 *
		 * unsyncronized, to be called from draw thread
		 */
		void invalidate();

		bool is3d() const;

		bool isCubeMap() const;

		bool isDepth() const;

		/**
		 * is texture current, (unused textures are deleted from opengl context)
		 *
		 * @return  true if texture current, false otherwise
		 */
		bool isEnabled() const;

		bool isFakeDepth() const;

		bool isFloatingPoint() const;

		/**
		 * resize texture
		 *
		 * unsynchronized, needs to be called in draw thread
		 *
		 * @param width   new texture width
		 * @param height  new texture height
		 */
		void resize(unsigned width, unsigned height);

		/**
		 * declare this texture a fake depth texture
		 *
		 * unsynchronized, expected to be called at texture construction
		 */
		void setFakeDepth();

		/**
		 * validate texture, if an image check its loaded before use
		 *
		 * synchronized, multiple render graph generation threads may be calling this
		 */
		bool validate();

		/**
		 * get script object factory for Texture
		 *
		 * @param currentDir  current directory of caller
		 *
		 * @return            Texture factory
		 */
		static ScriptObjectPtr getFactory(const std::string & currentDir);

	private:
		struct impl;
		std::unique_ptr<impl> pimpl;
	};
}

