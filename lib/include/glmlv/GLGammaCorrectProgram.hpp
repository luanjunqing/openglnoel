#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"

namespace glmlv
{

class GLGammaCorrectProgram: public GLProgram {
    GLint m_UniformInputImageLocation    = -1;
    GLint m_UniformOutputImageLocation   = -1;
    GLint m_UniformGammaExponentLocation = -1;

public:
    GLGammaCorrectProgram(const fs::path& cs):
        GLProgram(compileProgram({ cs.string() })),
        m_UniformInputImageLocation   (getUniformLocation("uInputImage")),
        m_UniformOutputImageLocation  (getUniformLocation("uOutputImage")),
        m_UniformGammaExponentLocation(getUniformLocation("uGammaExponent"))
        {}
    void setUniformInputImage(GLint i)      const { glUniform1i(m_UniformInputImageLocation, i); }
    void setUniformOutputImage(GLint i)     const { glUniform1i(m_UniformOutputImageLocation, i); }
    void setUniformGammaExponent(GLfloat f) const { glUniform1f(m_UniformGammaExponentLocation, f); }
};

} // namespace glmlv
