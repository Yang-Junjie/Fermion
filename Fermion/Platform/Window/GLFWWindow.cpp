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

        Init(props);
    }

    GLFWWindow::~GLFWWindow()
    {

        Shutdown();
    }

    void GLFWWindow::Init(const WindowProps &props)
    {

        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;

        if (s_GLFWWindowCount == 0)
        {
            int success = glfwInit();
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        {
            m_window = glfwCreateWindow(static_cast<int>(props.width),static_cast<int>(props.height), m_data.title.c_str(), nullptr, nullptr);
            if (m_window)
            {
                Log::Info("Window created: " + props.title);
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
        setVSync(true);

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
                    Log::Info("Key pressed: " + std::to_string(key));
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(GLFWKeyCodeToFMKeyCode(key));
					data.eventCallback(event);
                    Log::Info("Key released: " + std::to_string(key));
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), true);
					data.eventCallback(event);
                    Log::Info("GLFW_REPEAT: " + std::to_string(key));
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
                    Log::Info("Mouse button pressed: " + std::to_string(button));
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(GLFWMouseCodeToFMouseCode(button));
					data.eventCallback(event);
                    Log::Info("Mouse button released: " + std::to_string(button));
					break;
				}
			} });

        glfwSetScrollCallback(m_window, [](GLFWwindow *window, double xOffset, double yOffset)
                              {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.eventCallback(event);
            Log::Info("Mouse scrolled: " + std::to_string(xOffset) + ", " + std::to_string(yOffset)); });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow *window, double xPos, double yPos)
                                 {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.eventCallback(event); 
            Log::Trace("Mouse moved: " + std::to_string(xPos) + ", " + std::to_string(yPos)); });
    }

    void GLFWWindow::Shutdown()
    {

        glfwDestroyWindow(m_window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
        Log::Info("Window destroyed");
    }

    void GLFWWindow::OnUpdate()
    {
        m_context->swapBuffers();

        glfwPollEvents();
    }

    void GLFWWindow::setVSync(bool enabled)
    {

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