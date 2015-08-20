#include "fbo.h"

#include "glDebug.h"
#include "texture.h"

#include "../core/config.h"

#include <cassert>

#include <GL/glew.h>

using namespace render;

namespace {
	/**
	 * create frame buffer object, and bind
	 *
	 * @return frame buffer id
	 */
	GLuint createFramebuffer() {
		GLuint fb;
		glGenFramebuffersEXT(1, &fb);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
		assert(glIsFramebufferEXT(fb));
		return fb;
	}

	/**
	 * Create render buffer object, bind, set storage to depth, attach to depth
	 *
	 * @param width
	 *            width of depth storage
	 * @param height
	 *            height of depth storage
	 * @return render buffer id
	 */
	GLuint createDepthRenderbuffer(GLsizei width, GLsizei height) {
		GLuint rb;
		glGenRenderbuffersEXT(1, &rb);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
		assert(glIsRenderbufferEXT(rb));

		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width,
				height);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rb);

		return rb;
	}

	/**
	 * bind texture to first color component of active frame buffer object
	 *
	 * @param texture
	 *            texture to bind
	 */
	void bindTarget2d(const std::shared_ptr<Texture> & texture) {
		texture->bind();

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				texture->glTarget(), texture->glId(), 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		GlDebug::printOpenGLError();
	}

	/**
	 * bind texture to depth component of active frame buffer object
	 *
	 * @param texture
	 *            texture to bind
	 */
	void bindDepthTarget(const std::shared_ptr<Texture> & texture) {
		texture->bind();

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
				texture->glTarget(), texture->glId(), 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		GlDebug::printOpenGLError();
	}

	/**
	 * bind 3d texture to first color component of frame buffer object
	 *
	 * @param texture
	 *            texture to bind
	 */
	void bindTarget3d(const std::shared_ptr<Texture> & texture) {
		texture->bind();

		glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
				texture->glId(), 0);

		glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		GlDebug::printOpenGLError();
	}

	/**
	 * bind 3d textures layer to color components of frame buffer object
	 *
	 * @param textures
	 *            textures to bind
	 * @param layer
	 *            layer to use
	 */
	void bindTargets3d(const std::vector<std::shared_ptr<Texture>> & textures,
			GLint layer) {
		GLsizei nTargets = static_cast<GLsizei>(textures.size());
#ifdef WIN32
		GLenum * attachments = static_cast<GLenum *>(_malloca(sizeof(GLenum) * nTargets));
#else
		GLenum attachments[nTargets];
#endif

		int i = 0;
		for (const auto & texture : textures) {
			assert(texture->getType() == GL_TEXTURE_3D);
			texture->bind();
			attachments[i] = GL_COLOR_ATTACHMENT0_EXT + i;
			glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachments[i],
					texture->glTarget(), texture->glId(), 0, layer);
			++i;
		}

		glDrawBuffers(nTargets, attachments);
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		GlDebug::printOpenGLError();
	}

	/**
	 * bind 3d textures to color components of frame buffer object
	 *
	 * @param textures
	 *            textures to bind
	 */
	void bindTargets3d(const std::vector<std::shared_ptr<Texture>> & textures) {
		GLsizei nTargets = static_cast<GLsizei>(textures.size());
#ifdef WIN32
		GLenum * attachments = static_cast<GLenum *>(_malloca(sizeof(GLenum) * nTargets));
#else
		GLenum attachments[nTargets];
#endif

		int i = 0;
		for (const auto & texture : textures) {
			assert(texture->getType() == GL_TEXTURE_3D);
			texture->bind();
			attachments[i] = GL_COLOR_ATTACHMENT0_EXT + i;
			glFramebufferTextureEXT(GL_FRAMEBUFFER_EXT, attachments[i],
					texture->glId(), 0);
			++i;
		}

		glDrawBuffers(nTargets, attachments);
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		GlDebug::printOpenGLError();
	}

	/**
	 * bind 2d textures to color components of frame buffer object
	 *
	 * @param textures
	 *            textures to bind
	 */
	void bindTargets2d(const std::vector<std::shared_ptr<Texture>> & textures) {
		GLsizei nTargets = static_cast<GLsizei>(textures.size());
#ifdef WIN32
		GLenum * attachments = static_cast<GLenum *>(_malloca(sizeof(GLenum) * nTargets));
#else
		GLenum attachments[nTargets];
#endif

		int i = 0;
		for (auto texture : textures) {
			assert(texture->getType() != GL_TEXTURE_3D);
			texture->bind();
			attachments[i] = GL_COLOR_ATTACHMENT0_EXT + i;
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachments[i],
					texture->glTarget(), texture->glId(), 0);
			++i;
		}

		glDrawBuffers(nTargets, attachments);
		glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
		GlDebug::printOpenGLError();
	}

	/**
	 * validate frame buffer object
	 */
	void validateFbo() {
		int status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		assert(status == GL_FRAMEBUFFER_COMPLETE_EXT);
	}

}

namespace render {
	struct Fbo::impl {
		GLuint m_fb;
		GLuint m_rb;
		unsigned m_width;
		unsigned m_height;
		unsigned m_depth;
		bool m_isDepth;
		bool m_isFloat;
		size_t m_nTargets;

		/*
		 *
		 */
		impl(const std::shared_ptr<Texture> & texture) :
						m_fb(createFramebuffer()),
						m_width(texture->getWidth()),
						m_height(texture->getHeight()),
						m_depth(texture->getDepth()),
						m_isDepth(texture->isDepth()),
						m_isFloat(texture->isFloatingPoint()),
						m_nTargets(1) {

			if (texture->getType() == GL_TEXTURE_3D) {
				bindTarget3d(texture);
			} else if (m_isDepth) {
				bindDepthTarget(texture);
			} else {
				m_rb = createDepthRenderbuffer(m_width, m_height);

				bindTarget2d(texture);
			}

			validateFbo();
		}

		/*
		 *
		 */
		impl(const std::vector<std::shared_ptr<Texture>> & textures, int layer) :
						m_fb(createFramebuffer()),
						m_width(textures[0]->getWidth()),
						m_height(textures[0]->getHeight()),
						m_depth(textures[0]->getDepth()),
						m_isDepth(false),
						m_isFloat(textures[0]->isFloatingPoint()),
						m_nTargets(textures.size()) {
			// create render buffer
			m_rb = createDepthRenderbuffer(m_width, m_height);

			bindTargets3d(textures, layer);

			validateFbo();
		}

		/*
		 *
		 */
		impl(const std::vector<std::shared_ptr<Texture>> & textures) :
						m_fb(createFramebuffer()),
						m_width(textures[0]->getWidth()),
						m_height(textures[0]->getHeight()),
						m_depth(textures[0]->getDepth()),
						m_isDepth(false),
						m_isFloat(textures[0]->isFloatingPoint()),
						m_nTargets(textures.size()) {

			if (textures[0]->getType() == GL_TEXTURE_3D) {
				bindTargets3d(textures);
			} else {
				// create render buffer
				m_rb = createDepthRenderbuffer(m_width, m_height);

				bindTargets2d(textures);
			}

			validateFbo();
		}

		/*
		 *
		 */
		~impl() {
			// destroy framebuffer
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			glDeleteFramebuffersEXT(1, &m_fb);
			if (m_isDepth == false)
				glDeleteRenderbuffersEXT(1, &m_rb);
			glDrawBuffer(GL_BACK);
			glReadBuffer(GL_BACK);
		}

		/*
		 *
		 */
		bool update(const std::shared_ptr<Texture> & texture) {
			texture->bind();

			if (texture->getWidth() != m_width)
				return false;
			if (texture->getHeight() != m_height)
				return false;
			if (texture->getDepth() != m_depth) {
				return false;
			}
			if (texture->isDepth() != m_isDepth)
				return false;
			if (texture->isFloatingPoint() != m_isFloat)
				return false;

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fb);

			if (texture->getType() == GL_TEXTURE_3D) {
				bindTarget3d(texture);
			} else if (m_isDepth) {
				bindDepthTarget(texture);
			} else {
				bindTarget2d(texture);
			}

			validateFbo();

			return true;
		}

		/*
		 *
		 */
		bool update(const std::vector<std::shared_ptr<Texture>> & textures) {
			if (textures.size() != m_nTargets) {
				return false;
			}

			bool allOk = true;
			for (auto texture : textures) {
				texture->bind();

				allOk = allOk && (texture->getType() == GL_TEXTURE_3D);
				allOk = allOk && (texture->getWidth() != m_width);
				allOk = allOk && (texture->getHeight() != m_height);
				allOk = allOk && (texture->getDepth() != m_depth);
				allOk = allOk && (texture->isFloatingPoint() != m_isFloat);
			}
			if (allOk == false) {
				return false;
			}

			bindTargets2d(textures);

			validateFbo();

			return true;
		}
	};
}

/*
 *
 */
Fbo::Fbo(std::shared_ptr<Texture> const & texture) :
		pimpl(new impl(texture)) {
}

/*
 *
 */
Fbo::Fbo(std::vector<std::shared_ptr<Texture>> const & textures, int layer) :
		pimpl(new impl(textures, layer)) {
}

/*
 *
 */
Fbo::Fbo(std::vector<std::shared_ptr<Texture>> const & textures) :
		pimpl(new impl(textures)) {
}

/*
 *
 */
Fbo::~Fbo() {
}

/*
 *
 */
bool Fbo::update(std::shared_ptr<Texture> const & texture) {
	return pimpl->update(texture);
}

/*
 *
 */
bool Fbo::update(std::vector<std::shared_ptr<Texture>> const & textures) {
	return pimpl->update(textures);
}

/*
 *
 */
void Fbo::disable() {
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	if (Config::getInstance().getBoolean("headless") == false) {
		glDrawBuffer(GL_BACK);
		glReadBuffer(GL_BACK);
	}
}
