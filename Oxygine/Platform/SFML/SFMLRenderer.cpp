#include "SFMLRenderer.h"
#include <unordered_map>
namespace Oxygine
{
    SFMLRenderer::SFMLRenderer(sf::RenderWindow &win) : m_window(win) {}

    void SFMLRenderer::drawRect(const glm::vec2 &pos, const glm::vec2 &size, const glm::vec4 &color)
    {
        sf::RectangleShape rect({size.x, size.y});
        rect.setPosition(pos.x, pos.y);
        rect.setFillColor(sf::Color(
            static_cast<sf::Uint8>(color.x * 255),
            static_cast<sf::Uint8>(color.y * 255),
            static_cast<sf::Uint8>(color.z * 255),
            static_cast<sf::Uint8>(color.w * 255)));

        m_window.draw(rect);
    }
    void SFMLRenderer::drawImage(const std::string &texturePath, const glm::vec2 &pos)
    {
        if (!m_cache.contains(texturePath))
            m_cache[texturePath].loadFromFile(texturePath);
        sf::Sprite sprite(m_cache[texturePath]);
        sprite.setPosition(pos.x, pos.y);
        m_window.draw(sprite);
    }
}