#pragma once
#include "Renderer/IRenderer.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>

class SFMLRenderer : public IRenderer {
    sf::RenderWindow& m_window;
    std::unordered_map<std::string, sf::Texture> m_cache;
public:
    explicit SFMLRenderer(sf::RenderWindow& win);
    void drawImage(const std::string& texturePath, float x, float y) override;
};
