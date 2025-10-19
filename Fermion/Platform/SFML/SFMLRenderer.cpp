#include "SFMLRenderer.hpp"
#include <unordered_map>
#include "Core/Log.hpp"
namespace Fermion
{
    SFMLRenderer::SFMLRenderer(sf::RenderWindow &win) : m_window(win) {}

    void SFMLRenderer::drawRect(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color)
    {
        sf::RectangleShape rect({size.x, size.y});
        rect.setPosition({pos.x, pos.y});
        rect.setFillColor(sf::Color(
            static_cast<uint8_t>(color.x * 255),
            static_cast<uint8_t>(color.y * 255),
            static_cast<uint8_t>(color.z * 255),
            static_cast<uint8_t>(color.w * 255)));

        m_window.draw(rect);
        Log::Trace("Drawing rect at (" + std::to_string(pos.x) + "," + std::to_string(pos.y) + ") with size (" + std::to_string(size.x) + "," + std::to_string(size.y) + ")");
    }
    void SFMLRenderer::drawImage(const std::string &texturePath, const glm::vec2 &pos)
    {
        if (!m_cache.contains(texturePath))
        {
            
            if (!m_cache[texturePath].loadFromFile(texturePath))
            {
                Log::Error("Failed to load texture from file: " + texturePath);
                return;
            }
        }
        sf::Sprite sprite(m_cache[texturePath]);
        sprite.setPosition({pos.x, pos.y});
        m_window.draw(sprite);
        Log::Trace("Drawing image " + texturePath + " at (" + std::to_string(pos.x) + "," + std::to_string(pos.y) + ")");
    }
}