#include "Material.hpp"
#include "Project/Project.hpp"

namespace Fermion
{

    Material::Material()
        : DiffuseColor(1.0f), AmbientColor(0.0f),
          Type(MaterialType::Phong),
          Albedo(1.0f), Metallic(0.0f), Roughness(0.5f), AO(1.0f)
    {
    }

    // Legacy Phong setters
    void Material::setDiffuseColor(const glm::vec4 &color)
    {
        DiffuseColor = color;
    }

    void Material::setAmbientColor(const glm::vec4 &color)
    {
        AmbientColor = color;
    }

    void Material::setDiffuseTexture(AssetHandle textureHandle)
    {
        DiffuseTextureHandle = textureHandle;
    }

    // PBR setters
    void Material::setMaterialType(MaterialType type)
    {
        Type = type;
    }

    void Material::setAlbedo(const glm::vec3 &albedo)
    {
        Albedo = albedo;
    }

    void Material::setMetallic(float metallic)
    {
        Metallic = glm::clamp(metallic, 0.0f, 1.0f);
    }

    void Material::setRoughness(float roughness)
    {
        Roughness = glm::clamp(roughness, 0.0f, 1.0f);
    }

    void Material::setAO(float ao)
    {
        AO = glm::clamp(ao, 0.0f, 1.0f);
    }

    void Material::setAlbedoMap(AssetHandle textureHandle)
    {
        AlbedoMapHandle = textureHandle;
    }

    void Material::setNormalMap(AssetHandle textureHandle)
    {
        NormalMapHandle = textureHandle;
    }

    void Material::setMetallicMap(AssetHandle textureHandle)
    {
        MetallicMapHandle = textureHandle;
    }

    void Material::setRoughnessMap(AssetHandle textureHandle)
    {
        RoughnessMapHandle = textureHandle;
    }

    void Material::setAOMap(AssetHandle textureHandle)
    {
        AOMapHandle = textureHandle;
    }

    // Bind
    void Material::bind(const std::shared_ptr<Shader> &shader, int slot) const
    {
        if (Type == MaterialType::PBR)
        {
            bindPBR(shader, slot);
        }
        else
        {
            bindPhong(shader, slot);
        }
    }

    // Clone / Copy
    std::shared_ptr<Material> Material::clone() const
    {
        auto result = std::make_shared<Material>();
        result->copyFrom(*this);
        return result;
    }

    void Material::copyFrom(const Material &other)
    {
        DiffuseColor = other.DiffuseColor;
        AmbientColor = other.AmbientColor;
        DiffuseTextureHandle = other.DiffuseTextureHandle;

        Type = other.Type;
        Albedo = other.Albedo;
        Metallic = other.Metallic;
        Roughness = other.Roughness;
        AO = other.AO;

        AlbedoMapHandle = other.AlbedoMapHandle;
        NormalMapHandle = other.NormalMapHandle;
        MetallicMapHandle = other.MetallicMapHandle;
        RoughnessMapHandle = other.RoughnessMapHandle;
        AOMapHandle = other.AOMapHandle;
    }

    // Getters
    bool Material::hasTexture() const { return static_cast<uint64_t>(DiffuseTextureHandle) != 0; }
    const glm::vec4 &Material::getDiffuseColor() const { return DiffuseColor; }
    const glm::vec4 &Material::getAmbientColor() const { return AmbientColor; }
    AssetHandle Material::getDiffuseTexture() const { return DiffuseTextureHandle; }
    MaterialType Material::getType() const { return Type; }
    const glm::vec3 &Material::getAlbedo() const { return Albedo; }
    float Material::getMetallic() const { return Metallic; }
    float Material::getRoughness() const { return Roughness; }
    float Material::getAO() const { return AO; }
    
    AssetHandle Material::getAlbedoMap() const { return AlbedoMapHandle; }
    AssetHandle Material::getNormalMap() const { return NormalMapHandle; }
    AssetHandle Material::getMetallicMap() const { return MetallicMapHandle; }
    AssetHandle Material::getRoughnessMap() const { return RoughnessMapHandle; }
    AssetHandle Material::getAOMap() const { return AOMapHandle; }

    // Private binding helpers
    void Material::bindPhong(const std::shared_ptr<Shader> &shader, int slot) const
    {
        auto assetManager = Project::getRuntimeAssetManager();
        std::shared_ptr<Texture2D> texture = nullptr;
        
        if (static_cast<uint64_t>(DiffuseTextureHandle) != 0)
        {
            texture = assetManager->getAsset<Texture2D>(DiffuseTextureHandle);
        }
        
        bool hasTexture = texture && texture->isLoaded();
        shader->setBool("u_UseTexture", hasTexture);
        shader->setFloat4("u_Kd", DiffuseColor);
        shader->setFloat4("u_Ka", AmbientColor);

        if (hasTexture)
        {
            texture->bind(slot);
            shader->setInt("u_Texture", slot);
        }
    }

    void Material::bindPBR(const std::shared_ptr<Shader> &shader, int slot) const
    {
        shader->setFloat3("u_Material.albedo", Albedo);
        shader->setFloat("u_Material.metallic", Metallic);
        shader->setFloat("u_Material.roughness", Roughness);
        shader->setFloat("u_Material.ao", AO);

        auto assetManager = Project::getRuntimeAssetManager();
        int currentSlot = slot;

        auto bindMap = [&](AssetHandle handle, const char *uniformName, const char *useFlag)
        {
            if (static_cast<uint64_t>(handle) != 0)
            {
                auto texture = assetManager->getAsset<Texture2D>(handle);
                if (texture && texture->isLoaded())
                {
                    texture->bind(currentSlot);
                    shader->setInt(uniformName, currentSlot);
                    shader->setBool(useFlag, true);
                    currentSlot++;
                    return;
                }
            }
            shader->setBool(useFlag, false);
        };

        bindMap(AlbedoMapHandle, "u_AlbedoMap", "u_UseAlbedoMap");
        bindMap(NormalMapHandle, "u_NormalMap", "u_UseNormalMap");
        bindMap(MetallicMapHandle, "u_MetallicMap", "u_UseMetallicMap");
        bindMap(RoughnessMapHandle, "u_RoughnessMap", "u_UseRoughnessMap");
        bindMap(AOMapHandle, "u_AOMap", "u_UseAOMap");
    }

} // namespace Fermion
