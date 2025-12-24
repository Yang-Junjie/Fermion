#include "SubTexture2D.hpp"
namespace Fermion
{
    SubTexture2D::SubTexture2D(const std::shared_ptr<Texture2D> &texture, const glm::vec2 &min, const glm::vec2 &max)
        : m_texture(texture)
    {
        m_texture = texture;
        m_texCoords[0] = {min.x, min.y};
        m_texCoords[1] = {max.x, min.y};
        m_texCoords[2] = {max.x, max.y};
        m_texCoords[3] = {min.x, max.y};
    }
    std::shared_ptr<SubTexture2D> SubTexture2D::createFromCoords(const std::shared_ptr<Texture2D> &texture,
                                                                 const glm::vec2 &coords, const glm::vec2 &cellsize, const glm::vec2 &spriteSize)
    {
        const float texW = static_cast<float>(texture->getWidth());
        const float texH = static_cast<float>(texture->getHeight());

        const float px = coords.x * cellsize.x;
        const float py = coords.y * cellsize.y;
        const float pw = spriteSize.x * cellsize.x;
        const float ph = spriteSize.y * cellsize.y;

        // Inset by half a texel to avoid bleeding when camera moves (sub-pixel sampling)
        glm::vec2 min = { (px + 0.5f) / texW, (py + 0.5f) / texH };
        glm::vec2 max = { (px + pw - 0.5f) / texW, (py + ph - 0.5f) / texH };

        return std::make_shared<SubTexture2D>(texture, min, max);
    }
}
