#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "imgui-SFML.h"

namespace Fermion
{
    class ImGuiBackendSFML
    {
    public:
        static bool Init(sf::RenderWindow& window)
        {
            return ImGui::SFML::Init(window);
        }

        static void Shutdown(sf::RenderWindow& window)
        {
            ImGui::SFML::Shutdown(window);
        }

        static void BeginFrame(sf::RenderWindow& window, sf::Time dt)
        {
            ImGui::SFML::Update(window, dt);
        }

        static void EndFrame(sf::RenderWindow& window)
        {
            ImGui::SFML::Render(window);
        }
    };
}
