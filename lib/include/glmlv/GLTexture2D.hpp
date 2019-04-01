#pragma once

#include <glad/glad.h>

#include "filesystem.hpp"
#include "Image2DRGBA.hpp"

namespace glmlv
{

// A RAII wrapper around an OpenGL 2D texture object.
class GLTexture2D {
    GLuint m_GLId = 0;
public:
    GLuint glId() const { return m_GLId; }

    ~GLTexture2D() {
        glDeleteTextures(1, &m_GLId);
    }
    GLTexture2D(const GLTexture2D& v) = delete;
    GLTexture2D& operator=(const GLTexture2D& v) = delete;
    GLTexture2D(GLTexture2D&& o): m_GLId(o.m_GLId) { o.m_GLId = 0; }
    GLTexture2D& operator=(GLTexture2D&& o) { std::swap(m_GLId, o.m_GLId); return *this; }

    GLTexture2D() {
        glGenTextures(1, &m_GLId);
        assert(m_GLId);
    }
    GLTexture2D(const Image2DRGBA& img): GLTexture2D() { uploadImage(img); }
    GLTexture2D(const fs::path& path): GLTexture2D(readImage(path)) {}
    GLTexture2D(GLenum fmt, GLsizei w, GLsizei h) : GLTexture2D() {
        bind();
        glTexStorage2D(GL_TEXTURE_2D, 1, fmt, w, h);
    }
    void bind() const {
        glBindTexture(GL_TEXTURE_2D, m_GLId);
    }
    void uploadImage(const Image2DRGBA& img) const {
        bind();
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, img.width(), img.height());
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 
            0, 0, img.width(), img.height(),
            GL_RGBA, GL_UNSIGNED_BYTE, img.data()
        );
    }
	void setLodBias(GLfloat f) const {
		bind();
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, f);
	}
	void generateMipMap() const {
		bind();
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	void setAnisotropy(GLfloat f) const {
		bind();
		static const GLenum TEXTURE_MAX_ANISOTROPY_EXT = 0x84FE;
		glTexParameterf(GL_TEXTURE_2D, TEXTURE_MAX_ANISOTROPY_EXT, f);
	}
};

} // namespace glmlv
