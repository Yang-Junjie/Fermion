#pragma once
#include "fmpch.hpp"
#include "Renderer/Shader.hpp"
#include "../Texture/Texture.hpp"

namespace Fermion {
    enum class MaterialType {
        Phong,      // 传统Phong光照模型
        PBR         // PBR材质
    };

    class Material {
    public:
        Material()
            : DiffuseColor(1.0f), AmbientColor(0.0f), UseTexture(false),
              Type(MaterialType::Phong),
              Albedo(1.0f), Metallic(0.0f), Roughness(0.5f), AO(1.0f) {
        }

        // Legacy Phong setters
        void setDiffuseColor(const glm::vec4 &color) {
            DiffuseColor = color;
        }

        void setAmbientColor(const glm::vec4 &color) {
            AmbientColor = color;
        }

        void setTexture(std::unique_ptr<Texture2D> texture) {
            if (texture) {
                setTextureShared(std::shared_ptr<Texture2D>(std::move(texture)));
            } else {
                setTextureShared(nullptr);
            }
        }
        
        void setTextureShared(std::shared_ptr<Texture2D> texture) {
            DiffuseTextureShared = std::move(texture);
            UseTexture = (DiffuseTextureShared && DiffuseTextureShared->isLoaded());
        }

        // PBR setters
        void setMaterialType(MaterialType type) {
            Type = type;
        }

        void setAlbedo(const glm::vec3 &albedo) {
            Albedo = albedo;
        }

        void setMetallic(float metallic) {
            Metallic = glm::clamp(metallic, 0.0f, 1.0f);
        }

        void setRoughness(float roughness) {
            Roughness = glm::clamp(roughness, 0.0f, 1.0f);
        }

        void setAO(float ao) {
            AO = glm::clamp(ao, 0.0f, 1.0f);
        }

        void setAlbedoMap(std::unique_ptr<Texture2D> texture) {
            if (texture) {
                AlbedoMapShared = std::shared_ptr<Texture2D>(std::move(texture));
            } else {
                AlbedoMapShared.reset();
            }
        }
        
        void setAlbedoMapShared(std::shared_ptr<Texture2D> texture) {
            AlbedoMapShared = texture;
        }

        void setNormalMap(std::unique_ptr<Texture2D> texture) {
            if (texture) {
                NormalMapShared = std::shared_ptr<Texture2D>(std::move(texture));
            } else {
                NormalMapShared.reset();
            }
        }
        
        void setNormalMapShared(std::shared_ptr<Texture2D> texture) {
            NormalMapShared = texture;
        }

        void setMetallicMap(std::unique_ptr<Texture2D> texture) {
            if (texture) {
                MetallicMapShared = std::shared_ptr<Texture2D>(std::move(texture));
            } else {
                MetallicMapShared.reset();
            }
        }
        
        void setMetallicMapShared(std::shared_ptr<Texture2D> texture) {
            MetallicMapShared = texture;
        }

        void setRoughnessMap(std::unique_ptr<Texture2D> texture) {
            if (texture) {
                RoughnessMapShared = std::shared_ptr<Texture2D>(std::move(texture));
            } else {
                RoughnessMapShared.reset();
            }
        }
        
        void setRoughnessMapShared(std::shared_ptr<Texture2D> texture) {
            RoughnessMapShared = texture;
        }

        void setAOMap(std::unique_ptr<Texture2D> texture) {
            if (texture) {
                AOMapShared = std::shared_ptr<Texture2D>(std::move(texture));
            } else {
                AOMapShared.reset();
            }
        }
        
        void setAOMapShared(std::shared_ptr<Texture2D> texture) {
            AOMapShared = texture;
        }

        void bind(const std::shared_ptr<Shader> &shader, int slot = 0) const {
            if (Type == MaterialType::PBR) {
                bindPBR(shader, slot);
            } else {
                bindPhong(shader, slot);
            }
        }

        std::shared_ptr<Material> clone() const {
            auto result = std::make_shared<Material>();
            result->copyFrom(*this);
            return result;
        }

        void copyFrom(const Material &other) {
            DiffuseColor = other.DiffuseColor;
            AmbientColor = other.AmbientColor;
            DiffuseTextureShared = other.DiffuseTextureShared;
            UseTexture = (DiffuseTextureShared && DiffuseTextureShared->isLoaded());

            Type = other.Type;
            Albedo = other.Albedo;
            Metallic = other.Metallic;
            Roughness = other.Roughness;
            AO = other.AO;

            AlbedoMapShared = other.AlbedoMapShared;
            NormalMapShared = other.NormalMapShared;
            MetallicMapShared = other.MetallicMapShared;
            RoughnessMapShared = other.RoughnessMapShared;
            AOMapShared = other.AOMapShared;
        }

        // Getters
        bool hasTexture() const {
            return UseTexture;
        }

        const glm::vec4 &getDiffuseColor() const {
            return DiffuseColor;
        }

        const glm::vec4 &getAmbientColor() const {
            return AmbientColor;
        }

        Texture2D *getTexture() const {
            return DiffuseTextureShared.get();
        }

        MaterialType getType() const {
            return Type;
        }

        const glm::vec3 &getAlbedo() const {
            return Albedo;
        }

        float getMetallic() const {
            return Metallic;
        }

        float getRoughness() const {
            return Roughness;
        }

        float getAO() const {
            return AO;
        }

    private:
        void bindPhong(const std::shared_ptr<Shader> &shader, int slot) const {
            bool hasTexture = DiffuseTextureShared && DiffuseTextureShared->isLoaded();
            shader->setBool("u_UseTexture", hasTexture);
            shader->setFloat4("u_Kd", DiffuseColor);
            shader->setFloat4("u_Ka", AmbientColor);

            if (hasTexture) {
                DiffuseTextureShared->bind(slot);
                shader->setInt("u_Texture", slot);
            }
        }

        void bindPBR(const std::shared_ptr<Shader> &shader, int slot) const {
            shader->setFloat3("u_Material.albedo", Albedo);
            shader->setFloat("u_Material.metallic", Metallic);
            shader->setFloat("u_Material.roughness", Roughness);
            shader->setFloat("u_Material.ao", AO);

            int currentSlot = slot;
            
            // Albedo Map
            if (AlbedoMapShared && AlbedoMapShared->isLoaded()) {
                AlbedoMapShared->bind(currentSlot);
                shader->setInt("u_AlbedoMap", currentSlot);
                shader->setBool("u_UseAlbedoMap", true);
                currentSlot++;
            } else {
                shader->setBool("u_UseAlbedoMap", false);
            }

            // Normal Map
            if (NormalMapShared && NormalMapShared->isLoaded()) {
                NormalMapShared->bind(currentSlot);
                shader->setInt("u_NormalMap", currentSlot);
                shader->setBool("u_UseNormalMap", true);
                currentSlot++;
            } else {
                shader->setBool("u_UseNormalMap", false);
            }

            // Metallic Map
            if (MetallicMapShared && MetallicMapShared->isLoaded()) {
                MetallicMapShared->bind(currentSlot);
                shader->setInt("u_MetallicMap", currentSlot);
                shader->setBool("u_UseMetallicMap", true);
                currentSlot++;
            } else {
                shader->setBool("u_UseMetallicMap", false);
            }

            // Roughness Map
            if (RoughnessMapShared && RoughnessMapShared->isLoaded()) {
                RoughnessMapShared->bind(currentSlot);
                shader->setInt("u_RoughnessMap", currentSlot);
                shader->setBool("u_UseRoughnessMap", true);
                currentSlot++;
            } else {
                shader->setBool("u_UseRoughnessMap", false);
            }

            // AO Map
            if (AOMapShared && AOMapShared->isLoaded()) {
                AOMapShared->bind(currentSlot);
                shader->setInt("u_AOMap", currentSlot);
                shader->setBool("u_UseAOMap", true);
                currentSlot++;
            } else {
                shader->setBool("u_UseAOMap", false);
            }
        }

        // Material type
        MaterialType Type;

        glm::vec4 DiffuseColor; // Kd 漫反射
        glm::vec4 AmbientColor; // Ka 环境光
        bool UseTexture;
        std::shared_ptr<Texture2D> DiffuseTextureShared = nullptr;

        glm::vec3 Albedo;        
        float Metallic;         
        float Roughness;       
        float AO;               

        std::shared_ptr<Texture2D> AlbedoMapShared = nullptr;
        std::shared_ptr<Texture2D> NormalMapShared = nullptr;
        std::shared_ptr<Texture2D> MetallicMapShared = nullptr;
        std::shared_ptr<Texture2D> RoughnessMapShared = nullptr;
        std::shared_ptr<Texture2D> AOMapShared = nullptr;
    };
} // namespace Fermion
