#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"
#include "CommonLighting.hpp"

namespace glmlv
{

class GLDeferredShadingPassProgram: public GLCommonLightingProgram {
    const GLint m_UniformGPositionLocation        = -1;
    const GLint m_UniformGNormalLocation          = -1;
    const GLint m_UniformGAmbientLocation         = -1;
    const GLint m_UniformGDiffuseLocation         = -1;
    const GLint m_UniformGGlossyShininessLocation = -1;

public:
    GLDeferredShadingPassProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        GLCommonLightingProgram(),
        m_UniformGPositionLocation       (getUniformLocation("uGPosition")),
        m_UniformGNormalLocation         (getUniformLocation("uGNormal")),
        m_UniformGAmbientLocation        (getUniformLocation("uGAmbient")),
        m_UniformGDiffuseLocation        (getUniformLocation("uGDiffuse")),
        m_UniformGGlossyShininessLocation(getUniformLocation("uGGlossyShininess"))
        {}
    typedef CommonLighting LightingUniforms;
    void setUniformGPosition       (GLint i) const { glUniform1i(m_UniformGPositionLocation, i); }
    void setUniformGNormal         (GLint i) const { glUniform1i(m_UniformGNormalLocation, i); }
    void setUniformGAmbient        (GLint i) const { glUniform1i(m_UniformGAmbientLocation, i); }
    void setUniformGDiffuse        (GLint i) const { glUniform1i(m_UniformGDiffuseLocation, i); }
    void setUniformGGlossyShininess(GLint i) const { glUniform1i(m_UniformGGlossyShininessLocation, i); }
};

} // namespace glmlv
