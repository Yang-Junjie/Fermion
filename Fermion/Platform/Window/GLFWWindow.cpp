#include "GLFWWindow.hpp"

#include "Core/Log.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/MouseCodes.hpp"
#include "GLFWKeyCodes.hpp"
#include "GLFWMouseCodes.hpp"

#include "Events/ApplicationEvent.hpp"
#include "Events/Event.hpp"
#include "Events/MouseEvent.hpp"
#include "Events/KeyEvent.hpp"

namespace Fermion
{
    static uint8_t s_GLFWWindowCount = 0;

    static void GLFWErrorCallback(int error, const char *description)
    {
    }

    GLFWWindow::GLFWWindow(const WindowProps &props)
    {
        FM_PROFILE_FUNCTION();

        init(props);
    }

    GLFWWindow::~GLFWWindow()
    {
        FM_PROFILE_FUNCTION();

        shutdown();
    }

    void GLFWWindow::init(const WindowProps &props)
    {
        FM_PROFILE_FUNCTION();

        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;

        if (s_GLFWWindowCount == 0)
        {
            int success = glfwInit();
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        {
            m_window = glfwCreateWindow(static_cast<int>(props.width), static_cast<int>(props.height), m_data.title.c_str(), nullptr, nullptr);
            if (m_window)
            {
                Log::Info(std::format("Window created: {}", props.title));
            }
            else
            {
                Log::Error("Window creation failed");
            }
            ++s_GLFWWindowCount;
        }
        m_context = GraphicsContext::create(m_window);
        m_context->init();
        glfwSetWindowUserPointer(m_window, &m_data);

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow *window, int width, int height)
                                  {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.width = width;
			data.height = height;

			WindowResizeEvent event(width, height);
			data.eventCallback(event); });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow *window)
                                   {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.eventCallback(event); });

        glfwSetKeyCallback(m_window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                           {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), 0);
					data.eventCallback(event);
                    Log::Trace(std::format("Key pressed: {}", key));
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(GLFWKeyCodeToFMKeyCode(key));
					data.eventCallback(event);
                    Log::Trace(std::format("Key released: {}", key));
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), true);
					data.eventCallback(event);
                    Log::Trace(std::format("Key pressed (repeat): {}", key));
					break;
				}
			} });

        glfwSetCharCallback(m_window, [](GLFWwindow *window, unsigned int keycode)
                            {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(GLFWKeyCodeToFMKeyCode(keycode));
			data.eventCallback(event); });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow *window, int button, int action, int mods)
                                   {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(GLFWMouseCodeToFMouseCode(button));
					data.eventCallback(event);
                    Log::Trace(std::format("Mouse button pressed: {}", button));
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(GLFWMouseCodeToFMouseCode(button));
					data.eventCallback(event);
                    Log::Trace(std::format("Mouse button released: {}", button));
					break;
				}
			} });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset)
                              {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.eventCallback(event);
            Log::Trace(std::format("Mouse scrolled: {}, {}", xOffset, yOffset)); });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double xPos, double yPos)
                                 {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.eventCallback(event); 
            Log::Trace(std::format("Mouse moved: {}, {}", xPos, yPos));
        });
    }

    void GLFWWindow::shutdown()
    {
        FM_PROFILE_FUNCTION();

        glfwDestroyWindow(m_window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
        Log::Info("Window destroyed");
    }

    void GLFWWindow::onUpdate()
    {
        FM_PROFILE_FUNCTION();

        m_context->swapBuffers();

        glfwPollEvents();
    }

    void GLFWWindow::setVSync(bool enabled)
    {
        FM_PROFILE_FUNCTION();

        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_data.VSync = enabled;
    }

    bool GLFWWindow::isVSync() const
    {
        return m_data.VSync;
    }

}