#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"

namespace glmlv
{

class GLDisplayDepthMapProgram: public GLProgram {
    const GLint m_UniformGDepthLocation = -1;

public:
    GLDisplayDepthMapProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        m_UniformGDepthLocation(getUniformLocation("uGDepth"))
        {}

    void setUniformGDepth(GLuint i) const {
        glUniform1i(m_UniformGDepthLocation, i);
    }
};

} // namespace glmlv
