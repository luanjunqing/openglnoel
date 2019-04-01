#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"

namespace glmlv
{

class GLDirectionalSMProgram: public GLProgram {
    const GLint m_UniformDirLightViewProjMatrixLocation = -1;

public:
    GLDirectionalSMProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        m_UniformDirLightViewProjMatrixLocation(getUniformLocation("uDirLightViewProjMatrix"))
        {}

    void setUniformDirLightViewProjMatrix(const glm::mat4& m) const {
        glUniformMatrix4fv(m_UniformDirLightViewProjMatrixLocation, 1, GL_FALSE, &m[0][0]);
    }
};

} // namespace glmlv
