#pragma once

#include <glmlv/filesystem.hpp>
#include <glmlv/GLFWHandle.hpp>
#include <glmlv/GLForwardRenderingProgram.hpp>
#include <glmlv/GLSampler.hpp>
#include <glmlv/GLTexture2D.hpp>
#include <glmlv/Mesh.hpp>
#include <glmlv/Scene.hpp>
#include <glmlv/Camera.hpp>
#include <glmlv/GlobalWavPlayer.hpp>

class Application
{
public:
    Application(int argc, char** argv);

    int run();
private:
    const size_t m_nWindowWidth = 1280;
    const size_t m_nWindowHeight = 720;
    glmlv::GLFWHandle m_GLFWHandle{ m_nWindowWidth, m_nWindowHeight, "Forward Rendering" };
    const glmlv::fs::path m_AppPath;
    const std::string m_AppName;
    const glmlv::fs::path m_AssetsRootPath;
    const glmlv::fs::path m_ShadersRootPath;
    const glmlv::fs::path m_ForwardVsPath;
    const glmlv::fs::path m_ForwardFsPath;
    const glmlv::GLForwardRenderingProgram m_ForwardProgram;
    const glmlv::GLSampler m_Sampler;
    const glmlv::GLTexture2D m_CubeTex, m_SphereTex;
    const glmlv::Mesh m_Cube, m_Sphere;
    const glmlv::Scene m_Scene;
    glmlv::Camera m_ViewController;
    static std::string static_ImGuiIniFilename;
};
