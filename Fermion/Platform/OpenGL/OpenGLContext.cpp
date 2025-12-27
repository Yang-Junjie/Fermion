#include "fmpch.hpp"
#include "OpenGLContext.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Fermion {

OpenGLContext::OpenGLContext(GLFWwindow *windowHandle) : m_windowHandle(windowHandle) {
    Log::Info("OpenGL Context created successfully.");
}

void OpenGLContext::init() {
    FM_PROFILE_FUNCTION();

    glfwMakeContextCurrent(m_windowHandle);
    int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (!status)
        Log::Error("Failed to initialize Glad");
    else
        Log::Info("Glad Initialized");

    Log::Info("OpenGL Info:");
    Log::Info(std::format("  Vendor:{} ", std::string(reinterpret_cast<const char *>(glGetString(GL_VENDOR)))));
    Log::Info(std::format("  Renderer:{}", std::string(reinterpret_cast<const char *>(glGetString(GL_RENDERER)))));
    Log::Info(std::format("  Version:{}", std::string(reinterpret_cast<const char *>(glGetString(GL_VERSION)))));
    Log::Info(std::format("  GLSL:{}", std::string(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)))));
}

void OpenGLContext::swapBuffers() {
    FM_PROFILE_FUNCTION();

    glfwSwapBuffers(m_windowHandle);
}

} // namespace Fermion
