#pragma once

#include <cstdlib> // size_t
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>

#include "GLFWHandle.hpp"
#include "OpenSimplexNoise.hpp"

namespace glmlv {

// An enhanced ViewController that, given perspective parameters, can
// build a perspective projection matrix.
struct Camera {

    static void checkForwardVector(glm::vec3& v) {
        const auto forward = glm::normalize(v);
        const auto up = getNormalizedUpVector();
        if(fabsf(glm::dot(forward, up)) > 0.9998f) {
            std::clog << "WARN: forward and up vectors are colinear! (NOTE: " << v << " vs. " << up << ")" << std::endl;
            // v = glm::vec3(glm::rotate(glm::mat4(1), glm::radians(1.f), glm::vec3(1,0,0)) * glm::vec4(v, 0.f));
        }
    }

    enum class Mode {
        FreeFly,
        LookAt,
    };
    struct LookAtData;
    struct FreeFlyData {
        glm::vec3 m_Position, m_Forward;

        FreeFlyData() : m_Position(0,0,0), m_Forward(0,0,-1) {}
        FreeFlyData(const LookAtData& l): m_Position(l.getPosition()), m_Forward(l.m_Forward) {}
        glm::mat4 getViewMatrix() const {
            checkForwardVector(const_cast<glm::vec3&>(m_Forward));
            return glm::lookAt(m_Position, m_Position + m_Forward, getNormalizedUpVector());
        }
    };
    struct LookAtData {
        glm::vec3 m_Target, m_Forward;

        LookAtData() : m_Target(0,0,-1), m_Forward(0,0,-1) {}
        LookAtData(const FreeFlyData& f): m_Target(f.m_Position + f.m_Forward), m_Forward(f.m_Forward) {}
        glm::vec3 getPosition() const {
            return m_Target - m_Forward;
        }
        glm::mat4 getViewMatrix() const {
            checkForwardVector(const_cast<glm::vec3&>(m_Forward));
            return glm::lookAt(getPosition(), m_Target, getNormalizedUpVector());
        }
    };

private:
    GLFWwindow* m_pWindow;
    bool m_LeftButtonPressed = false;
    glm::dvec2 m_LastCursorPosition = glm::dvec2(0);
    Mode m_Mode;
    OpenSimplexNoise m_OpenSimplexNoise;
    glm::dvec2 m_CurrentNoiseValue = glm::dvec2(0);
    double m_TotalTime = 0.0;
public:
    size_t m_nWindowWidth, m_nWindowHeight;
    float m_FovY, m_Near, m_Far, m_Speed;
    FreeFlyData m_FreeFlyData;
    LookAtData m_LookAtData;
    glm::vec2 m_NoiseFactor;
    float m_NoiseSpeed;

    Camera() = delete;
    Camera(GLFWwindow* window, size_t w, size_t h, float speed = 1):
        m_pWindow(window),
        m_Mode(Mode::FreeFly),
        m_OpenSimplexNoise(),
        m_nWindowWidth(w),
        m_nWindowHeight(h),
        m_FovY(glm::radians(60.f)),
        m_Near(0.2f),
        m_Far(10000.0f),
        m_Speed(speed),
        m_FreeFlyData(),
        m_LookAtData(m_FreeFlyData),
        m_NoiseFactor(0),
        m_NoiseSpeed(1)
        {}

    Mode getMode() const { return m_Mode; }
    void setMode(Mode mode) {
        if(m_Mode == mode)
            return;
        switch(mode) {
        case Mode::FreeFly: m_FreeFlyData = FreeFlyData(m_LookAtData); break;
        case Mode::LookAt: m_LookAtData = LookAtData(m_FreeFlyData); break;
        }
        m_Mode = mode;
    }

    float getAspect() const {
        assert(m_nWindowWidth);
        assert(m_nWindowHeight);
        return m_nWindowWidth / float(m_nWindowHeight);
    }
    glm::mat4 getProjMatrix() const {
        assert(m_FovY > 0);
        assert(m_FovY < glm::radians(180.f));
        return glm::perspective(m_FovY, getAspect(), m_Near, m_Far);
    }
    glm::vec3 getNoiseTranslationVector() const {
        const auto tx = float(m_CurrentNoiseValue.x * m_NoiseFactor.x) * getNormalizedRightVector();
        const auto ty = float(m_CurrentNoiseValue.y * m_NoiseFactor.y) * getNormalizedUpVector();
        return tx + ty;
    }
    glm::mat4 getViewMatrix() const {
        return glm::translate(getViewMatrixWithoutNoise(), getNoiseTranslationVector());
    }
    glm::mat4 getViewMatrixWithoutNoise() const {
        switch(m_Mode) {
        case Mode::FreeFly: return m_FreeFlyData.getViewMatrix(); break;
        case Mode::LookAt: return m_LookAtData.getViewMatrix(); break;
        }
        throw std::runtime_error("This line is unreachable!");
    }
    glm::mat4 getRcpViewMatrix() const {
        return glm::inverse(getViewMatrix());
    }
    static glm::vec3 getNormalizedUpVector() {
        return glm::vec3(0,1,0);
    }
    glm::vec3 getUnnormalizedForwardVector() const {
        switch(m_Mode) {
        case Mode::FreeFly: return m_FreeFlyData.m_Forward;
        case Mode::LookAt: return m_LookAtData.m_Forward;
        }
        throw std::runtime_error("This line is unreachable!");
    }
    glm::vec3 getNormalizedForwardVector() const {
        return glm::normalize(getUnnormalizedForwardVector());
    }
    glm::vec3 getNormalizedLeftVector() const {
        return glm::cross(getNormalizedUpVector(), getNormalizedForwardVector());
    }
    glm::vec3 getNormalizedRightVector() const {
        return -getNormalizedLeftVector();
    }
    glm::vec3 getPosition() const {
        switch(m_Mode) {
        case Mode::FreeFly: return m_FreeFlyData.m_Position;
        case Mode::LookAt: return m_LookAtData.getPosition();
        }
        throw std::runtime_error("This line is unreachable!");
    }
    glm::vec3 getTarget() const {
        switch(m_Mode) {
        case Mode::FreeFly: return m_FreeFlyData.m_Position + m_FreeFlyData.m_Forward;
        case Mode::LookAt: return m_LookAtData.m_Target;
        }
        throw std::runtime_error("This line is unreachable!");
    }
    float getDistanceFromTarget() const {
        return glm::length(getUnnormalizedForwardVector());
    }

    void update(float dt) {

        m_TotalTime += dt * m_NoiseSpeed;

        m_CurrentNoiseValue = glm::dvec2(
            m_OpenSimplexNoise.eval(m_TotalTime, 0.0),
            m_OpenSimplexNoise.eval(0.0, m_TotalTime)
        );

        const auto forward = getNormalizedForwardVector();
        const auto right = getNormalizedRightVector();
        const auto up = getNormalizedUpVector();

        if (glfwGetKey(m_pWindow, GLFW_KEY_W)) {
            m_FreeFlyData.m_Position += m_Speed * dt * forward;
            const auto candidate = m_LookAtData.m_Forward - m_Speed * dt * forward;
            if(glm::dot(glm::normalize(candidate), glm::normalize(m_LookAtData.m_Forward)) > 0.9f)
                m_LookAtData.m_Forward = candidate;
            else
                m_LookAtData.m_Forward = glm::normalize(m_LookAtData.m_Forward) * m_Near;
        }
        if (glfwGetKey(m_pWindow, GLFW_KEY_S)) {
            m_FreeFlyData.m_Position -= m_Speed * dt * forward;
            m_LookAtData.m_Forward += m_Speed * dt * forward;
        }
        if (glfwGetKey(m_pWindow, GLFW_KEY_A))
            m_FreeFlyData.m_Position -= m_Speed * dt * right;
        if (glfwGetKey(m_pWindow, GLFW_KEY_D))
            m_FreeFlyData.m_Position += m_Speed * dt * right;
        if (glfwGetKey(m_pWindow, GLFW_KEY_UP))
            m_FreeFlyData.m_Position += m_Speed * dt * up;
        if (glfwGetKey(m_pWindow, GLFW_KEY_DOWN))
            m_FreeFlyData.m_Position -= m_Speed * dt * up;

        if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && !m_LeftButtonPressed) {
            m_LeftButtonPressed = true;
            glfwGetCursorPos(m_pWindow, &m_LastCursorPosition.x, &m_LastCursorPosition.y);
        } else if (!glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT) && m_LeftButtonPressed) {
            m_LeftButtonPressed = false;
        }

        if (m_LeftButtonPressed) {
            glm::dvec2 cursorPosition;
            glfwGetCursorPos(m_pWindow, &cursorPosition.x, &cursorPosition.y);
            glm::dvec2 delta = cursorPosition - m_LastCursorPosition;
            m_LastCursorPosition = cursorPosition;

            const auto ry = glm::rotate(glm::mat4(1), -0.01f * float(delta.x), getNormalizedUpVector());
            const auto rx = glm::rotate(glm::mat4(1), -0.01f * float(delta.y), getNormalizedRightVector());
            const auto m = ry * rx;
            const auto apply = [](const glm::mat4& m, glm::vec3& v) {
                v = glm::vec3(m * glm::vec4(v, 0));
            };
            apply(m, m_FreeFlyData.m_Forward);
            apply(m, m_LookAtData.m_Forward);
        }
    }

    // Compatibility with ViewController
    void setSpeed(float speed) { m_Speed = speed; }
    float getSpeed() const { return m_Speed; }
};

} // namespace glmlv
