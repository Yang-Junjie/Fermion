#pragma once
#include "Texture.hpp"
#include <glm/glm.hpp>
namespace Fermion
{
    class SubTexture2D
    {
    public:
        SubTexture2D(const std::shared_ptr<Texture2D> &texture, const glm::vec2 &min, const glm::vec2 &max);
        const glm::vec2 *getTexCoords() const
        {
            return m_texCoords;
        }
        const std::shared_ptr<Texture2D> &getTexture() const
        {
            return m_texture;
        }

        static std::shared_ptr<SubTexture2D> createFromCoords(const std::shared_ptr<Texture2D> &texture,
                                                              const glm::vec2 &coords,const glm::vec2& cellsize, const glm::vec2 &spriteSize = {1, 1});

    private:
        std::shared_ptr<Texture2D> m_texture;
        glm::vec2 m_texCoords[4];
    };

}