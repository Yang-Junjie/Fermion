#include "Platform/SFMLRenderer.h"
#include <unordered_map>
namespace Oxygine
{
    SFMLRenderer::SFMLRenderer(sf::RenderWindow &win) : m_window(win) {}

    void SFMLRenderer::drawImage(const std::string &texturePath, float x, float y)
    {
        if (!m_cache.contains(texturePath))
            m_cache[texturePath].loadFromFile(texturePath);
        sf::Sprite sprite(m_cache[texturePath]);
        sprite.setPosition(x, y);
        m_window.draw(sprite);
    }
}