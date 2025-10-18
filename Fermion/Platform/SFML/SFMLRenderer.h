#pragma once
#include "Renderer/Renderer.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
namespace Fermion
{
    class SFMLRenderer : public IRenderer
    {
        sf::RenderWindow &m_window;
        std::unordered_map<std::string, sf::Texture> m_cache;

    public:
        explicit SFMLRenderer(sf::RenderWindow &win);
        void drawRect(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color) override;
        void drawImage(const std::string &texturePath, const glm::vec2 &pos) override;
    };
}