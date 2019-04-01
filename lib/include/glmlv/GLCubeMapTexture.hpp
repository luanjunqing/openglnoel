#pragma once

#include <glad/glad.h>

#include "filesystem.hpp"
#include "Image2DRGBA.hpp"

namespace glmlv
{

struct CubeMapFacePaths {
    fs::path nx, px, py, ny, pz, nz;
};

struct CubeMapFaceImages {
    Image2DRGBA nx, px, py, ny, pz, nz;

    CubeMapFaceImages(): 
        nx(),
        px(),
        py(),
        ny(),
        pz(),
        nz()
        {}

    CubeMapFaceImages(const CubeMapFacePaths& paths): 
        nx(readImage(paths.nx)),
        px(readImage(paths.px)),
        py(readImage(paths.py)),
        ny(readImage(paths.ny)),
        pz(readImage(paths.pz)),
        nz(readImage(paths.nz))
        {}
};

// A RAII wrapper around an OpenGL Cube Map texture object.
class GLCubeMapTexture {
    GLuint m_GLId = 0;

public:
    GLuint glId() const { return m_GLId; }

    ~GLCubeMapTexture() {
        glDeleteTextures(1, &m_GLId);
    }
    GLCubeMapTexture(const GLCubeMapTexture& v) = delete;
    GLCubeMapTexture& operator=(const GLCubeMapTexture& v) = delete;
    GLCubeMapTexture(GLCubeMapTexture&& o): m_GLId(o.m_GLId) { o.m_GLId = 0; }
    GLCubeMapTexture& operator=(GLCubeMapTexture&& o) { std::swap(m_GLId, o.m_GLId); return *this; }

    GLCubeMapTexture() {
        glGenTextures(1, &m_GLId);
        assert(m_GLId);
    }
    GLCubeMapTexture(const CubeMapFaceImages& faces): GLCubeMapTexture() { uploadImages(faces); }
    GLCubeMapTexture(const CubeMapFacePaths& paths): GLCubeMapTexture(CubeMapFaceImages(paths)) {}
    void bind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_GLId);
    }
    void uploadImages(const CubeMapFaceImages& f) const {
        bind();
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, f.px.width(), f.px.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, f.px.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, f.py.width(), f.py.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, f.py.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, f.pz.width(), f.pz.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, f.pz.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, f.nx.width(), f.nx.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, f.nx.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, f.ny.width(), f.ny.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, f.ny.data());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, f.nz.width(), f.nz.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, f.nz.data());
    }
	void setLodBias(GLfloat f) const {
		bind();
		glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, f);
	}
	void generateMipMap() const {
		bind();
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	void setAnisotropy(GLfloat f) const {
		bind();
		static const GLenum TEXTURE_MAX_ANISOTROPY_EXT = 0x84FE;
		glTexParameterf(GL_TEXTURE_CUBE_MAP, TEXTURE_MAX_ANISOTROPY_EXT, f);
	}
};

} // namespace glmlv
