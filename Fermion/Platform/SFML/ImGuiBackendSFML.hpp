#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "imgui-SFML.h"
#include "Core/Timestep.hpp"
#include "ImGui/ImGuiBackend.hpp"

namespace Fermion
{

    class ImGuiBackendSFMLImpl : public IImGuiBackend
    {
    public:
        ImGuiBackendSFMLImpl(sf::RenderWindow &window)
            : m_window(window) {}

        bool Init(void *nativeWindow) override
        {
            return ImGui::SFML::Init(m_window);
        }

        void BeginFrame(Timestep dt) override
        {
            ImGui::SFML::Update(m_window, sf::seconds(dt.GetSeconds()));
        }

        void EndFrame() override
        {
            ImGui::SFML::Render(m_window);
        }

        void Shutdown() override
        {
            ImGui::SFML::Shutdown(m_window);
        }

    private:
        sf::RenderWindow &m_window;
    };

} // namespace Fermion