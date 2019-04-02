#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLProgram.hpp>
#include <glmlv/ViewController.hpp>
#include <glmlv/simple_geometry.hpp>
#include <glm/glm.hpp>
#include <limits>
#include <tiny_gltf.h>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    static glm::vec3 computeDirectionVector(float phiRadians, float thetaRadians)
    {
        const auto cosPhi = glm::cos(phiRadians);
        const auto sinPhi = glm::sin(phiRadians);
        const auto sinTheta = glm::sin(thetaRadians);
        return glm::vec3(sinPhi * sinTheta, glm::cos(thetaRadians), cosPhi * sinTheta);
    }

    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ int(m_nWindowWidth), int(m_nWindowHeight), "glTF viewer" }; // Note: the handle must be declared before the creation of any object managing OpenGL resource (e.g. GLProgram, GLShader)

    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const std::string m_ImGuiIniFilename;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_AssetsRootPath;

    GLuint m_textureSampler = 0;

    glmlv::GLProgram m_program;

    glmlv::ViewController m_viewController{m_GLFWHandle.window(), 3.0f};
    GLint m_uModelViewProjMatrixLocation;
    GLint m_uModelViewMatrixLocation;
    GLint m_uNormalMatrixLocation;

    GLint m_uDirectionalLightDirLocation;
    GLint m_uDirectionalLightIntensityLocation;

    GLint m_uPointLightPositionLocation;
    GLint m_uPointLightIntensityLocation;

    GLint m_uKdLocation;
    GLint m_uKdSamplerLocation;

    float m_DirLightPhiAngleDegrees = 100.f;
    float m_DirLightThetaAngleDegrees = 45.f;
    glm::vec3 m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
    glm::vec3 m_DirLightColor = glm::vec3(1, 1, 1);
    float m_DirLightIntensity = 1.0f;

    glm::vec3 m_PointLightPosition = glm::vec3(0, 10, 0);
    glm::vec3 m_PointLightColor = glm::vec3(1, 1, 1);
    float m_PointLightIntensity = 5.0f;


    std::string path;
    glm::mat4 m_projMatrix;
    glm::mat4 m_viewMatrix;

    // 使用gluint映射tinyglTF缓冲区类型以应用于正确的缓冲区
    std::map<std::string, GLuint> m_attribs;

    std::vector<GLuint> m_vaos;
    std::vector<tinygltf::Primitive> m_primitives;
    std::vector<GLuint> m_diffuseTex;

    tinygltf::Model m_model;

    // 加载glTF 使用tinyglTF库
    void loadModel();

    void DrawNode(tinygltf::Model &model, const tinygltf::Node &node, glm::mat4 m);
    // 绘制场景网格
    void DrawMesh(int meshIndex, glm::mat4 modelMatrix);

    GLenum getglTFMode(int mode);
    void drawModel(tinygltf::Model &model);
};