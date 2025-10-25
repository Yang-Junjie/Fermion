#include "fmpch.hpp"
#include "OpenGLContext.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Fermion
{

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
        : m_windowHandle(windowHandle)
    {
        Log::Info("OpenGL Context created successfully.");
    }

    void OpenGLContext::init()
    {

        glfwMakeContextCurrent(m_windowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!status)
            Log::Error("Failed to initialize Glad");
        else
            Log::Info("Glad Initialized");

       
        Log::Info("OpenGL Info:");
        Log::Info("  Vendor: " + std::string(reinterpret_cast<const char *>(glGetString(GL_VENDOR))));
        Log::Info("  Renderer: " + std::string(reinterpret_cast<const char *>(glGetString(GL_RENDERER))));
        Log::Info("  Version: " + std::string(reinterpret_cast<const char *>(glGetString(GL_VERSION))));
        Log::Info("  GLSL: " + std::string(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION))));

    }

    void OpenGLContext::swapBuffers()
    {
        glfwSwapBuffers(m_windowHandle);
    }

}
