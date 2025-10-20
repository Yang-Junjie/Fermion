#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "imgui-SFML.h"
#include "Core/Timestep.hpp"

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

        static void BeginFrame(sf::RenderWindow& window, Timestep dt)
        {
            ImGui::SFML::Update(window, sf::seconds(dt.GetSeconds()));
        }

        static void EndFrame(sf::RenderWindow& window)
        {
            ImGui::SFML::Render(window);
        }

    };
}
