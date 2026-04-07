#include "Core/Log.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/Event.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "GLFWKeyCodes.hpp"
#include "GLFWMouseCodes.hpp"
#include "GLFWWindow.hpp"
#include "Renderer/RendererAPI.hpp"

namespace Fermion {
static uint8_t s_GLFWWindowCount = 0;

static void GLFWErrorCallback(int error, const char* description)
{
    Log::Error(std::format("GLFW Error {}: {}", error, description ? description : "Unknown"));
}

GLFWWindow::GLFWWindow(const WindowProps& props)
{
    FM_PROFILE_FUNCTION();

    init(props);
}

GLFWWindow::~GLFWWindow()
{
    FM_PROFILE_FUNCTION();

    shutdown();
}

void GLFWWindow::init(const WindowProps& props)
{
    FM_PROFILE_FUNCTION();

    m_data.title = props.title;
    m_data.width = props.width;
    m_data.height = props.height;
    m_data.VSync = false;

    if (s_GLFWWindowCount == 0) {
        int success = glfwInit();
        FERMION_ASSERT(success == GLFW_TRUE, "Failed to initialize GLFW");
        glfwSetErrorCallback(GLFWErrorCallback);
    }

    glfwDefaultWindowHints();
    switch (RendererAPI::getAPI()) {
        case RendererAPI::API::OpenGL:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            break;
        case RendererAPI::API::Vulkan:
        case RendererAPI::API::None:
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
    }

    if (props.maximized) {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    }

    m_window = glfwCreateWindow(static_cast<int>(props.width),
                                static_cast<int>(props.height),
                                m_data.title.c_str(),
                                nullptr,
                                nullptr);
    if (!m_window) {
        Log::Error("Window creation failed");
        return;
    }

    Log::Info(std::format("Window created: {}", props.title));
    ++s_GLFWWindowCount;

    m_context = GraphicsContext::create(m_window);
    if (m_context) {
        m_context->init();
    }

    glfwSetWindowUserPointer(m_window, this);

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        self->m_data.width = width;
        self->m_data.height = height;

        if (self->m_context) {
            self->m_context->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        }

        WindowResizeEvent event(width, height);
        self->m_data.eventCallback(event);
    });

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
        WindowCloseEvent event;
        self->m_data.eventCallback(event);
    });

    glfwSetKeyCallback(m_window,
                       [](GLFWwindow* window, int key, int scancode, int action, int mods) {
                           auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

                           switch (action) {
                               case GLFW_PRESS: {
                                   KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), 0);
                                   self->m_data.eventCallback(event);
                                   Log::Trace(std::format("Key pressed: {}", key));
                                   break;
                               }
                               case GLFW_RELEASE: {
                                   KeyReleasedEvent event(GLFWKeyCodeToFMKeyCode(key));
                                   self->m_data.eventCallback(event);
                                   Log::Trace(std::format("Key released: {}", key));
                                   break;
                               }
                               case GLFW_REPEAT: {
                                   KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), true);
                                   self->m_data.eventCallback(event);
                                   Log::Trace(std::format("Key pressed (repeat): {}", key));
                                   break;
                               }
                           }
                       });

    glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode) {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

        KeyTypedEvent event(GLFWKeyCodeToFMKeyCode(keycode));
        self->m_data.eventCallback(event);
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

        switch (action) {
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(GLFWMouseCodeToFMouseCode(button));
                self->m_data.eventCallback(event);
                Log::Trace(std::format("Mouse button pressed: {}", button));
                break;
            }
            case GLFW_RELEASE: {
                MouseButtonReleasedEvent event(GLFWMouseCodeToFMouseCode(button));
                self->m_data.eventCallback(event);
                Log::Trace(std::format("Mouse button released: {}", button));
                break;
            }
        }
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

        MouseScrolledEvent event((float) xOffset, (float) yOffset);
        self->m_data.eventCallback(event);
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos) {
        auto* self = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));

        MouseMovedEvent event((float) xPos, (float) yPos);
        self->m_data.eventCallback(event);
    });
}

void GLFWWindow::shutdown()
{
    FM_PROFILE_FUNCTION();

    m_context.reset();

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        --s_GLFWWindowCount;
    }

    if (s_GLFWWindowCount == 0) {
        glfwTerminate();
    }
    Log::Info("Window destroyed");
}

void GLFWWindow::onUpdate()
{
    FM_PROFILE_FUNCTION();

    if (m_context) {
        m_context->present();
    }

    glfwPollEvents();
}

void GLFWWindow::setVSync(bool enabled)
{
    FM_PROFILE_FUNCTION();

    if (m_context) {
        m_context->setVSync(enabled);
    }

    m_data.VSync = enabled;
}

void GLFWWindow::getWindowPos(int* x, int* y) const
{
    glfwGetWindowPos(m_window, x, y);
}

void GLFWWindow::setWindowPos(int x, int y)
{
    glfwSetWindowPos(m_window, x, y);
}

void GLFWWindow::setMaximized()
{
    glfwMaximizeWindow(m_window);
}

void GLFWWindow::setRestored()
{
    glfwRestoreWindow(m_window);
}

void GLFWWindow::setMinimized()
{
    glfwIconifyWindow(m_window);
}

bool GLFWWindow::isVSync() const
{
    return m_data.VSync;
}

DeviceInfo GLFWWindow::getDeviceInfo() const
{
    if (!m_context) {
        return {};
    }

    return m_context->getDeviceInfo();
}

} // namespace Fermion
