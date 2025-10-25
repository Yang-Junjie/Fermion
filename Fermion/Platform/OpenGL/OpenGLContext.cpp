#include "fmpch.hpp"
#include "OpenGLContext.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Fermion
{

    OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
        : m_WindowHandle(windowHandle)
    {
        Log::Info("OpenGL Context created");
    }

    void OpenGLContext::init()
    {

        glfwMakeContextCurrent(m_WindowHandle);
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        Log::Debug("Glad status " + std::to_string(status));

        // Log::Info("OpenGL Info:");
       
    }

    void OpenGLContext::swapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }

}
