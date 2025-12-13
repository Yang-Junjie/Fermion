#pragma once
#include "fmpch.hpp"
#include "Renderer/Shader.hpp"
#include "Texture.hpp"

namespace Fermion
{
    class Material
    {
    public:
        Material()
            : DiffuseColor(1.0f), AmbientColor(0.0f), UseTexture(false) {}

        void setDiffuseColor(const glm::vec4 &color)
        {
            DiffuseColor = color;
        }

        void setAmbientColor(const glm::vec4 &color)
        {
            AmbientColor = color;
        }

        void setTexture(const std::shared_ptr<Texture2D> &texture)
        {
            DiffuseTexture = texture;
            UseTexture = (texture && texture->isLoaded());
        }

        void bind(const std::shared_ptr<Shader> &shader, int slot = 0) const
        {
            shader->setBool("u_UseTexture", UseTexture);
            shader->setFloat4("u_Kd", DiffuseColor);
            shader->setFloat4("u_Ka", AmbientColor);

            if (UseTexture)
            {
                DiffuseTexture->bind(slot);
                shader->setInt("u_Texture", slot);
            }
        }

        bool hasTexture() const { return UseTexture; }
        const glm::vec4 &getDiffuseColor() const { return DiffuseColor; }
        const glm::vec4 &getAmbientColor() const { return AmbientColor; }
        std::shared_ptr<Texture2D> getTexture() const { return DiffuseTexture; }

    private:
        glm::vec4 DiffuseColor;  // Kd 漫反射
        glm::vec4 AmbientColor;  // Ka 环境光
        bool UseTexture;
        std::shared_ptr<Texture2D> DiffuseTexture;
    };
}
