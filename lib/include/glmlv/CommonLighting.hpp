#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "Camera.hpp"

namespace glmlv
{

// XXX: I'm not actually of fan of forcing implicit world-space to view-space mapping
// when uploading uniforms
struct CommonLighting {
    // NOTE: Keep in sync with forward shader
    static const size_t MAX_POINT_LIGHTS = 32;

    glm::vec3 dirLightDir = glm::vec3(1,0,0);
    glm::vec3 dirLightIntensity = glm::vec3(1,1,1);
    GLfloat dirLightShadowMapBias = 0;
    GLint dirLightShadowMap = 0;
    GLint dirLightShadowMapSampleCount = 1;
    GLfloat dirLightShadowMapSpread = 0.0005;
    glm::mat4 dirLightViewProjMatrix = glm::mat4(1);
    int pointLightCount = 1; // int to be editable via GUI
    glm::vec3 pointLightPosition[MAX_POINT_LIGHTS] = {};
    glm::vec3 pointLightIntensity[MAX_POINT_LIGHTS] = {};
    GLfloat pointLightRange[MAX_POINT_LIGHTS] = {};
    GLfloat pointLightAttenuationFactor[MAX_POINT_LIGHTS] = {};
};


class GLCommonLightingProgram: public virtual GLProgram {
    const GLint m_UniformDirectionalLightDirLocation         = -1;
    const GLint m_UniformDirectionalLightIntensityLocation   = -1;
    const GLint m_UniformPointLightPositionLocation          = -1;
    const GLint m_UniformPointLightIntensityLocation         = -1;
    const GLint m_UniformPointLightRangeLocation             = -1;
    const GLint m_UniformPointLightAttenuationFactorLocation = -1;
    const GLint m_UniformPointLightCountLocation             = -1;

    const GLint m_UniformDirLightViewProjMatrix               = -1;
    const GLint m_UniformDirLightShadowMap                    = -1;
    const GLint m_UniformDirLightShadowMapBias                = -1;
    const GLint m_UniformDirLightShadowMapSampleCountLocation = -1;
    const GLint m_UniformDirLightShadowMapSpreadLocation      = -1;

public:
    GLCommonLightingProgram():
        m_UniformDirectionalLightDirLocation          (getUniformLocation("uDirectionalLightDir")),
        m_UniformDirectionalLightIntensityLocation    (getUniformLocation("uDirectionalLightIntensity")),
        m_UniformPointLightPositionLocation           (getUniformLocation("uPointLightPosition")),
        m_UniformPointLightIntensityLocation          (getUniformLocation("uPointLightIntensity")),
        m_UniformPointLightRangeLocation              (getUniformLocation("uPointLightRange")),
        m_UniformPointLightAttenuationFactorLocation  (getUniformLocation("uPointLightAttenuationFactor")),
        m_UniformPointLightCountLocation              (getUniformLocation("uPointLightCount")),
        m_UniformDirLightViewProjMatrix               (getUniformLocation("uDirLightViewProjMatrix")),
        m_UniformDirLightShadowMap                    (getUniformLocation("uDirLightShadowMap")),
        m_UniformDirLightShadowMapBias                (getUniformLocation("uDirLightShadowMapBias")),
        m_UniformDirLightShadowMapSampleCountLocation (getUniformLocation("uDirLightShadowMapSampleCount")),
        m_UniformDirLightShadowMapSpreadLocation      (getUniformLocation("uDirLightShadowMapSpread"))
        {}

    typedef CommonLighting LightingUniforms;
    static const size_t MAX_POINT_LIGHTS = LightingUniforms::MAX_POINT_LIGHTS;

    void setLightingUniforms(const CommonLighting& d, const Camera& vc) const {
        size_t count = d.pointLightCount;
        assert(count < MAX_POINT_LIGHTS);
        setUniformDirectionalLightDir(d.dirLightDir, vc);
        setUniformDirectionalLightIntensity(d.dirLightIntensity);
        setUniformDirLightViewProjMatrix(d.dirLightViewProjMatrix);
        setUniformDirLightShadowMap(d.dirLightShadowMap);
        setUniformDirLightShadowMapBias(d.dirLightShadowMapBias);
        setUniformDirLightShadowMapSampleCount(d.dirLightShadowMapSampleCount);
        setUniformDirLightShadowMapSpread(d.dirLightShadowMapSpread);
        setUniformPointLightCount(count);
        setUniformPointLightPosition(count, d.pointLightPosition, vc);
        setUniformPointLightIntensity(count, d.pointLightIntensity);
        setUniformPointLightRange(count, d.pointLightRange);
        setUniformPointLightAttenuationFactor(count, d.pointLightAttenuationFactor);
    }

    void setUniformDirectionalLightDir(const glm::vec3& v, const Camera& vc) const {
        auto view = vc.getViewMatrix();
        auto data = glm::vec3(view * glm::vec4(v, 0));
        glUniform3fv(m_UniformDirectionalLightDirLocation, 1, &data[0]);
    }
    void setUniformDirectionalLightIntensity(const glm::vec3& v) const {
        glUniform3fv(m_UniformDirectionalLightIntensityLocation, 1, &v[0]);
    }

    void setUniformDirLightViewProjMatrix(const glm::mat4 &m) const {
        glUniformMatrix4fv(m_UniformDirLightViewProjMatrix, 1, GL_FALSE, &m[0][0]);
    }
    void setUniformDirLightShadowMap(GLint i) const {
        glUniform1i(m_UniformDirLightShadowMap, i);
    }
    void setUniformDirLightShadowMapBias(GLfloat bias) const {
        glUniform1f(m_UniformDirLightShadowMapBias, bias);
    }
    void setUniformDirLightShadowMapSampleCount(GLint i) const {
        glUniform1i(m_UniformDirLightShadowMapSampleCountLocation, i);
    }
    void setUniformDirLightShadowMapSpread(GLfloat v) const {
        glUniform1f(m_UniformDirLightShadowMapSpreadLocation, v);
    }


    void setUniformPointLightPosition(size_t count, const glm::vec3* v, const Camera& vc) const {
        assert(count < MAX_POINT_LIGHTS);
        auto view = vc.getViewMatrix();
        glm::vec3 vsPointLightPosition[MAX_POINT_LIGHTS] = {};
        for(size_t i=0 ; i<MAX_POINT_LIGHTS ; ++i) {
            vsPointLightPosition[i] = glm::vec3(view * glm::vec4(v[i], 1));
        }
        glUniform3fv(m_UniformPointLightPositionLocation, count, &vsPointLightPosition[0][0]);
    }
    void setUniformPointLightIntensity(size_t count, const glm::vec3* v) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform3fv(m_UniformPointLightIntensityLocation, count, &v[0][0]);
    }
    void setUniformPointLightRange(size_t count, const GLfloat* factor) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform1fv(m_UniformPointLightRangeLocation, count, factor);
    }
    void setUniformPointLightAttenuationFactor(size_t count, const GLfloat* factor) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform1fv(m_UniformPointLightAttenuationFactorLocation, count, factor);
    }
    void setUniformPointLightCount(size_t count) const {
        assert(count < MAX_POINT_LIGHTS);
        glUniform1ui(m_UniformPointLightCountLocation, count);
    }
};

} // namespace glmlv
