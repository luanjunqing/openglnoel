#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "Mesh.hpp"
#include "GLSampler.hpp"
#include "GLForwardRenderingProgram.hpp"
#include "GLDeferredGPassProgram.hpp"
#include "GLTexture2D.hpp"
#include "Camera.hpp"
#include "simple_geometry.hpp"

namespace glmlv
{

struct SceneInstanceData {
    glm::vec3 m_Position;
    glm::vec3 m_Forward = glm::vec3(0,0,-1);
    glm::vec3 m_Scale = glm::vec3(1);

    glm::mat4 getTranslationMatrix() const {
        return glm::translate(glm::mat4(1), m_Position);
    }
    glm::mat4 getRotationMatrix() const {
        return glm::mat4_cast(quatLookAt(m_Forward, glm::vec3(0,1,0)));
    }
    glm::mat4 getScaleMatrix() const {
        return glm::scale(glm::mat4(1), m_Scale);
    }
    glm::mat4 getModelMatrix() const {
        return getTranslationMatrix() * getRotationMatrix() * getScaleMatrix();
    }

    // https://github.com/g-truc/glm/pull/659/commits/3ee83a15ef776cd9db9af29d3426bb9d7c39e6a2
    static glm::quat quatLookAt(const glm::vec3& direction, const glm::vec3& up) {
        using namespace glm;
        mat3 Result;
        Result[2] = -normalize(direction);
        Result[0] = normalize(cross(up, Result[2]));
        Result[1] = cross(Result[2], Result[0]);
        return quat_cast(Result);
    }
};

// A scene loaded from an OBJ file; Contains both the on-CPU and on-GPU data.
struct Scene {

    static constexpr bool WANTS_TEXTURES_BY_DEFAULT = 
#if 0 // Change this to speed up program loading time.
        false
#else
        true
#endif
        ;

    ObjData m_ObjData;
    GLMesh m_GLMesh;
    GLSampler m_GLSampler;
    std::vector<GLTexture2D> m_GLTextures2D;

    Scene() = delete;
    Scene(const fs::path& path, bool wantsTextures = WANTS_TEXTURES_BY_DEFAULT):
        m_ObjData(objDataFromPath(path, wantsTextures)),
        m_GLMesh(SimpleGeometry { m_ObjData.vertexBuffer, m_ObjData.indexBuffer }),
        m_GLSampler(GLSamplerParams().withWrapST(GL_REPEAT).withMinMagFilter(GL_LINEAR))
    {
        for(const auto& img: m_ObjData.textures) {
            m_GLTextures2D.emplace_back(img);
			m_GLTextures2D.back().generateMipMap();
			m_GLTextures2D.back().setLodBias(0.0f);
			m_GLTextures2D.back().setAnisotropy(4.0f);
        }
        std::clog << "Done loading " << path << ": Loaded " << m_GLTextures2D.size() << " textures into GL." << std::endl;
    }
	glm::vec3 getBboxSize() const {
		return m_ObjData.bboxMax - m_ObjData.bboxMin;
	}
    float getDiagonalLength() const {
        return glm::length(getBboxSize());
    }

    void render(const GLMaterialProgram& prog, const Camera& camera, const SceneInstanceData& instance) const {
        const auto& view = camera.getViewMatrix();
        const auto& proj = camera.getProjMatrix();
        glm::mat4 modelView(view * instance.getModelMatrix());
        glm::mat4 modelViewProj(proj * modelView);
        glm::mat4 normalMatrix(transpose(inverse(modelView)));

        prog.setUniformModelViewProjMatrix(modelViewProj);
        prog.setUniformModelViewMatrix(modelView);
        prog.setUniformNormalMatrix(normalMatrix);

        for(GLuint i=0 ; i<m_GLTextures2D.size() ; ++i) {
            glActiveTexture(GL_TEXTURE0 + i);
            m_GLTextures2D[i].bind();
            m_GLSampler.bindToTextureUnit(i);
        }

        glBindVertexArray(m_GLMesh.getVao());

        auto indexOffset = 0u;
        for(auto i=0u ; i < m_ObjData.shapeCount ; ++i) {
            const auto matId = m_ObjData.materialIDPerShape[i];
            ObjData::PhongMaterial mat; // Have a default one...
            if(matId >= 0) { // ...In case matId == -1
                mat = m_ObjData.materials[matId];
            }
            prog.setUniformKa(mat.Ka);
            prog.setUniformKd(mat.Kd);
            prog.setUniformKs(mat.Ks);
            prog.setUniformShininess(mat.shininess);
            prog.setUniformKaSampler(mat.KaTextureId >= 0 ? mat.KaTextureId : 0);
            prog.setUniformKdSampler(mat.KdTextureId >= 0 ? mat.KdTextureId : 0);
            prog.setUniformKsSampler(mat.KsTextureId >= 0 ? mat.KsTextureId : 0);
            prog.setUniformShininessSampler(mat.shininessTextureId >= 0 ? mat.shininessTextureId : 0);

            const auto indexCount = m_ObjData.indexCountPerShape[i];
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        }
    }

    void render() const {
        glBindVertexArray(m_GLMesh.getVao());
        auto indexOffset = 0u;
        for(auto i=0u ; i < m_ObjData.shapeCount ; ++i) {
            const auto indexCount = m_ObjData.indexCountPerShape[i];
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*) (indexOffset * sizeof(GLuint)));
            indexOffset += indexCount;
        }
    }
private:
    static ObjData objDataFromPath(const fs::path& path, bool wantsTextures = true) {
        ObjData d;
        loadObj(path, d, wantsTextures);
        return d;
    }
};

} // namespace glmlv
