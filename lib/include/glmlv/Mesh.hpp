#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GLForwardRenderingProgram.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "simple_geometry.hpp"

namespace glmlv
{

struct MeshInstanceData {
    glm::mat4 modelMatrix = glm::mat4(1);
    glm::vec3 color = glm::vec3(1,1,1);
    GLuint textureUnit = 0;
};

// A class that groups together a VAO, VBO and EBO.
// In essence, it is the on-GPU equivalent of SimpleGeometry.
// The render() method simply binds the VAO and calls glDrawElements().
class GLMesh {
    GLuint m_Vao = 0, m_Vbo = 0, m_Ibo = 0;
    GLsizei m_ElementCount = 0;
public:
    GLuint getVao() const { return m_Vao; }
    GLuint getVbo() const { return m_Vbo; }
    GLuint getIbo() const { return m_Ibo; }
    GLuint getElementCount() const { return m_ElementCount; }

    ~GLMesh() {
        glDeleteBuffers(1, &m_Ibo);
        glDeleteBuffers(1, &m_Vbo);
        glDeleteVertexArrays(1, &m_Vao);
    }
    GLMesh(const GLMesh& v) = delete;
    GLMesh& operator=(const GLMesh& v) = delete;
    GLMesh(GLMesh&& o): m_Vao(o.m_Vao), m_Vbo(o.m_Vbo), m_Ibo(o.m_Ibo), m_ElementCount(o.m_ElementCount) {
        o.m_Vao = o.m_Vbo = o.m_Ibo = 0;
    }
    GLMesh& operator=(GLMesh&& o) {
        std::swap(m_Vao, o.m_Vao);
        std::swap(m_Vbo, o.m_Vbo);
        std::swap(m_Ibo, o.m_Ibo);
        std::swap(m_ElementCount, o.m_ElementCount);
        return *this;
    }

    GLMesh() = delete;
    GLMesh(const SimpleGeometry& mesh) {
        m_ElementCount = mesh.indexBuffer.size();
        GLuint bufs[2] = {0, 0};
        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(2, bufs);
        m_Vbo = bufs[0];
        m_Ibo = bufs[1];
        assert(m_Vao);
        assert(m_Vbo);
        assert(m_Ibo);
        const auto& v = mesh.vertexBuffer;
        const auto& i = mesh.indexBuffer;
        glBindVertexArray(m_Vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof v[0], v.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, i.size() * sizeof i[0], i.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        const auto stride = sizeof mesh.vertexBuffer[0];
        static_assert(std::is_same<const Vertex3f3f2f&, decltype(mesh.vertexBuffer[0])>::value, "Vertex types don't match!");
        static_assert(sizeof mesh.indexBuffer[0] == sizeof(uint32_t), "We are expecting to use GL_UNSIGNED_INT for indices!");
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, normal));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*) offsetof(Vertex3f3f2f, texCoords));
    }
    void render() const {
        glBindVertexArray(m_Vao);
        glDrawElements(GL_TRIANGLES, m_ElementCount, GL_UNSIGNED_INT, nullptr);
    }
    void render(const GLMaterialProgram& prog, const Camera& camera, const MeshInstanceData& i) const {
        const auto& view = camera.getViewMatrix();
        const auto& proj = camera.getProjMatrix();
        glm::mat4 modelView(view * i.modelMatrix);
        glm::mat4 modelViewProj(proj * modelView);
        glm::mat4 normalMatrix(transpose(inverse(modelView)));

		prog.resetMaterialUniforms();
        prog.setUniformKdSampler(i.textureUnit);
        prog.setUniformKd(i.color);
        prog.setUniformModelViewProjMatrix(modelViewProj);
        prog.setUniformModelViewMatrix(modelView);
        prog.setUniformNormalMatrix(normalMatrix);
        
        render(); // PERF: Binds VAO as many times as there are instances; useless.
    }
};

// A struct that groups a SimpleGeometry and GLMesh together; Essentially,
// both the on-CPU and on-GPU data for a mesh.
struct Mesh {
    SimpleGeometry m_SimpleGeometry;
    GLMesh m_GLMesh;
    Mesh() = delete;
    Mesh(const SimpleGeometry& mesh) : m_SimpleGeometry(mesh), m_GLMesh(mesh) {}
    void render() const {
        m_GLMesh.render();
    }
    void render(const GLMaterialProgram& prog, const Camera& camera, const MeshInstanceData& i) const {
        m_GLMesh.render(prog, camera, i);
    }
};

} // namespace glmlv
