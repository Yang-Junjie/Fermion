#pragma once

#include "Core/Layer.hpp"
#include "Events/Event.hpp"
#include "Renderer/RendererAPI.hpp"

namespace Fermion
{
    class IWindow;

    class ImGuiLayer : public Layer
    {
    public:
        explicit ImGuiLayer(IWindow &window);

        ~ImGuiLayer() override = default;

        void onAttach() override;

        void onDetach() override;

        void onEvent(IEvent &e) override;

        void onImGuiRender() override;

        void begin();

        void end();

        void blockEvents(const bool block)
        {
            m_blockEvents = block;
        }
        void setImGuiWidgetStyle();
        void setDarkThemeColors();

    private:
        bool m_blockEvents = true;
        IWindow &m_window;
        void *m_nativeWindow = nullptr;
        RendererAPI::API m_rendererAPI = RendererAPI::API::None;
        bool m_platformBackendInitialized = false;
        bool m_rendererBackendInitialized = false;
    };
} // namespace Fermion
