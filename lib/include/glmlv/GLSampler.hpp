#pragma once

#include <cassert>
#include <glad/glad.h>

namespace glmlv
{

// Pretty self-explanatory; Helper struct for initializing sampler parameters.
struct GLSamplerParams {
    GLenum m_WrapS = GL_REPEAT;
    GLenum m_WrapT = GL_REPEAT;
    GLenum m_MinFilter = GL_LINEAR;
    GLenum m_MagFilter = GL_LINEAR;
    GLSamplerParams withWrapST(GLenum v) const {
        auto p = *this;
        p.m_WrapS = p.m_WrapT = v;
        return p;
    }
    GLSamplerParams withMinMagFilter(GLenum v) const {
        auto p = *this;
        p.m_MinFilter = p.m_MagFilter = v;
        return p;
    }
};

// A RAII wrapper around an OpenGL sampler object.
class GLSampler {
    GLuint m_GLId = 0;
public:
    GLuint glId() const { return m_GLId; }

    ~GLSampler() {
        glDeleteSamplers(1, &m_GLId);
    }
    GLSampler(const GLSampler& v) = delete;
    GLSampler& operator=(const GLSampler& v) = delete;
    GLSampler(GLSampler&& o): m_GLId(o.m_GLId) { o.m_GLId = 0; }
    GLSampler& operator=(GLSampler&& o) { std::swap(m_GLId, o.m_GLId); return *this; }

    GLSampler() = delete;
    GLSampler(const GLSamplerParams& p) {
        glGenSamplers(1, &m_GLId);
        assert(m_GLId);
        glSamplerParameteri(m_GLId, GL_TEXTURE_WRAP_S, p.m_WrapS);
        glSamplerParameteri(m_GLId, GL_TEXTURE_WRAP_T, p.m_WrapT);
        glSamplerParameteri(m_GLId, GL_TEXTURE_MIN_FILTER, p.m_MinFilter);
        glSamplerParameteri(m_GLId, GL_TEXTURE_MAG_FILTER, p.m_MagFilter);
    }
    void bindToTextureUnit(GLuint unit) const {
        glBindSampler(unit, m_GLId);
    }
};

} // namespace glmlv
