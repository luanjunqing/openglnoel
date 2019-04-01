#pragma once

#include <glmlv/glfw.hpp>
#include <glmlv/gl_debug_output.hpp>
#include <glmlv/imgui_impl_glfw_gl3.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>

namespace glmlv
{

// Class responsible for initializing GLFW, creating a window, initializing OpenGL function pointers with GLAD library and initializing ImGUI
class GLFWHandle
{

    static void glfwErrorHandler(int code, const char *msg) {
        std::cerr << "GLFW error " << code << ": " << msg << std::endl;
    }

public:
    GLFWHandle(size_t width, size_t height, const char * title)
    {
        glfwSetErrorCallback(glfwErrorHandler);

        if (!glfwInit()) {
            std::cerr << "Unable to init GLFW.\n";
            throw std::runtime_error("Unable to init GLFW.\n");
        }

        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);

        const int profiles[] = {GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_COMPAT_PROFILE};
        for(const auto profile: profiles) {
            glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
            for(int minor=5 ; minor>=0 ; --minor) {
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
                m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
                if (m_pWindow)
                    break;
                std::cerr << "Could not open an OpenGL 4." << minor << "-enabled window." << std::endl;
            }
            if (m_pWindow)
                break;
        }
        if (!m_pWindow) {
            std::cerr << "Unable to open window.\n";
            glfwTerminate();
            throw std::runtime_error("Unable to open window.\n");
        }

        glfwMakeContextCurrent(m_pWindow);

        const char* vsync_str = std::getenv("VSYNC");
        int vsync_setting = vsync_str ? atoi(vsync_str) : 1;
        std::cout << "Calling glfwSwapInterval(" << vsync_setting << ") " << (vsync_str ? "(from VSYNC env var)" : "(VSYNC env var wasn't set)") << std::endl;
        glfwSwapInterval(vsync_setting);

        if (!gladLoadGL()) {
            std::cerr << "Unable to init OpenGL.\n";
            throw std::runtime_error("Unable to init OpenGL.\n");
        }

        GLint ctxflags, ctxpflags, depth_bits, stencil_bits;
        GLboolean double_buffer, stereo_buffers;

        glGetIntegerv(GL_CONTEXT_FLAGS, &ctxflags);
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &ctxpflags);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_DEPTH, 
                GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_bits);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_STENCIL, 
                GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencil_bits);
        glGetBooleanv(GL_DOUBLEBUFFER, &double_buffer);
        glGetBooleanv(GL_STEREO, &stereo_buffers);


        printf(
            "--- Active OpenGL context settings ---\n"
            "    Version             : %s\n"
            "    Renderer            : %s\n"
            "    Vendor              : %s\n"
            "    GLSL version        : %s\n"
            "    Profile flags       : %s%s(%#x)\n"
            "    Context flags       : %s%s(%#x)\n"
            "    Double buffering    : %s\n"
            "    Stereo buffers      : %s\n"
            "    Depth buffer bits   : %d\n"
            "    Stencil buffer bits : %d\n"
            "\n",
            glGetString(GL_VERSION),
            glGetString(GL_RENDERER),
            glGetString(GL_VENDOR),
            glGetString(GL_SHADING_LANGUAGE_VERSION),
            ctxpflags & GL_CONTEXT_CORE_PROFILE_BIT ? "core " : "",
            ctxpflags & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT ? "compatibility " :"",
            ctxpflags,
            ctxflags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT ? "forward_compatible " : "",
            ctxflags & GL_CONTEXT_FLAG_DEBUG_BIT ? "debug " : "",
            ctxflags,
            double_buffer ? "yes" : "no",
            stereo_buffers ? "yes" : "no",
            depth_bits,
            stencil_bits
        );

        if(!std::getenv("GL_DONT_DEBUG")) {
            std::cout << "NOTE: Enabling debug output (OpenGL 4.3+). Set GL_DONT_DEBUG environment variable to disable." << std::endl;
            glmlv::initGLDebugOutput();
        }

        // Setup ImGui binding
        ImGui_ImplGlfwGL3_Init(m_pWindow, true);
    }

    ~GLFWHandle()
    {
        ImGui_ImplGlfwGL3_Shutdown();
        glfwTerminate();
    }

    // Non-copyable class:
    GLFWHandle(const GLFWHandle&) = delete;
    GLFWHandle& operator =(const GLFWHandle&) = delete;

    bool shouldClose() const
    {
        return glfwWindowShouldClose(m_pWindow);
    }

    glm::ivec2 framebufferSize() const
    {
        int displayWidth, displayHeight;
        glfwGetFramebufferSize(m_pWindow, &displayWidth, &displayHeight);
        return glm::ivec2(displayWidth, displayHeight);
    }

    void swapBuffers() const
    {
        glfwSwapBuffers(m_pWindow);
    }

    GLFWwindow* window()
    {
        return m_pWindow;
    }

private:
    GLFWwindow* m_pWindow = nullptr;
};

}
