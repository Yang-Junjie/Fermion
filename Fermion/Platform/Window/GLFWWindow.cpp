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

        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        if (s_GLFWWindowCount == 0)
        {

            int success = glfwInit();

            glfwSetErrorCallback(GLFWErrorCallback);
        }

        {

            m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
            glfwMakeContextCurrent(m_Window);
            auto status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
            if (!status)
            {
                Log::Error("Failed to initialize GLFW!");
            }

            ++s_GLFWWindowCount;
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);
        setVSync(true);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
                                  {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Width = width;
			data.Height = height;

			WindowResizeEvent event(width, height);
			data.EventCallback(event); });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window)
                                   {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
            Log::Info("Window closed"); });

        glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                           {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), 0);
					data.EventCallback(event);
                    Log::Info("Key pressed: " + std::to_string(key));
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(GLFWKeyCodeToFMKeyCode(key));
					data.EventCallback(event);
                    Log::Info("Key released: " + std::to_string(key));
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(GLFWKeyCodeToFMKeyCode(key), true);
					data.EventCallback(event);
                    Log::Info("GLFW_REPEAT: " + std::to_string(key));
					break;
				}
			} });

        glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int keycode)
                            {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(GLFWKeyCodeToFMKeyCode(keycode));
			data.EventCallback(event); });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods)
                                   {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(GLFWMouseCodeToFMouseCode(button));
					data.EventCallback(event);
                    Log::Info("Mouse button pressed: " + std::to_string(button));
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(GLFWMouseCodeToFMouseCode(button));
					data.EventCallback(event);
                    Log::Info("Mouse button released: " + std::to_string(button));
					break;
				}
			} });

        glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double xOffset, double yOffset)
                              {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
            Log::Info("Mouse scrolled: " + std::to_string(xOffset) + ", " + std::to_string(yOffset)); });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xPos, double yPos)
                                 {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event); 
            Log::Trace("Mouse moved: " + std::to_string(xPos) + ", " + std::to_string(yPos)); });
    }

    void GLFWWindow::Shutdown()
    {

        glfwDestroyWindow(m_Window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
        Log::Info("Window destroyed");
    }

    void GLFWWindow::OnUpdate()
    {
        glfwSwapBuffers(m_Window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
    }

    void GLFWWindow::setVSync(bool enabled)
    {

        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool GLFWWindow::isVSync() const
    {
        return m_Data.VSync;
    }

}