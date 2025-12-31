#include "Core/Application.hpp"
#include "Core/Timestep.hpp"
#include "Script/ScriptManager.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Model/MeshFactory.hpp"
#include "Renderer/Renderer.hpp"
#include <GLFW/glfw3.h>

namespace Fermion {
    Application *Application::s_instance = nullptr;

    Application::Application(const ApplicationSpecification &spec) {
        FM_PROFILE_FUNCTION();
        s_instance = this;

        Renderer::setConfig(spec.rendererConfig);
        WindowProps windowProps;
        windowProps.title = spec.name;
        windowProps.width = spec.windowWidth;
        windowProps.height = spec.windowHeight;

        m_window = IWindow::create(windowProps);
        m_window->setEventCallback([this](IEvent &event) { this->onEvent(event); });
        m_window->setVSync(false);
        Renderer::init();
        ScriptManager::init();
        MeshFactory::Init();
        m_imGuiLayer = std::make_unique<ImGuiLayer>(m_window->getNativeWindow());
        m_imGuiLayerRaw = m_imGuiLayer.get();
        pushOverlay(std::move(m_imGuiLayer));
    }

    Application::~Application() {
        FM_PROFILE_FUNCTION();
        ScriptManager::shutdown();
    }

    void Application::run() {
        FM_PROFILE_FUNCTION();
        Log::Info("Application started!");

        while (m_running) {
            FM_PROFILE_SCOPE("RunLoop");
            const float time = static_cast<float>(glfwGetTime()); // TODO :GLFE TIMER
            const Timestep timestep = time - m_lastFrameTime;
            m_lastFrameTime = time;

            if (!m_minimized) {
                {
                    FM_PROFILE_SCOPE("LayerStack OnUpdate");
                    for (auto &layer: m_layerStack)
                        layer->onUpdate(timestep);
                }
                m_imGuiLayerRaw->begin();
                {
                    FM_PROFILE_SCOPE("LayerStack OnImGuiRender");
                    for (auto &layer: m_layerStack)
                        layer->onImGuiRender();
                }
                m_imGuiLayerRaw->end();
            }
            m_window->onUpdate();
        }
    }

    void Application::pushLayer(std::unique_ptr<Layer> layer) {
        FM_PROFILE_FUNCTION();
        layer->onAttach();
        m_layerStack.pushLayer(std::move(layer));
    }

    void Application::pushOverlay(std::unique_ptr<Layer> overlay) {
        FM_PROFILE_FUNCTION();
        overlay->onAttach();
        m_layerStack.pushOverlay(std::move(overlay));
    }

    void Application::onEvent(IEvent &event) {
        FM_PROFILE_FUNCTION();
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent &e) { return this->onWindowResize(e); });
        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent &e) { return this->onWindowClose(e); });

        for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
            if (event.handled)
                break;
            (*it)->onEvent(event);
        }
    }

    bool Application::onWindowResize(const WindowResizeEvent &event) {
        FM_PROFILE_FUNCTION();
        if (event.getWidth() == 0 || event.getHeight() == 0) {
            m_minimized = true;
            return false;
        }
        m_minimized = false;
        Renderer::onWindowResize(event.getWidth(), event.getHeight());
        return true;
    }

    bool Application::onWindowClose(const WindowCloseEvent &event) {
        m_running = false;
        Log::Info("Window closed");
        return true;
    }
} // namespace Fermion
