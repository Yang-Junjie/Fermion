#pragma once
#include "fmpch.hpp"
#include "Renderer/Shader.hpp"
#include "Asset/Asset.hpp"

namespace Fermion
{

    enum class MaterialType : uint8_t
    {
        Phong = 1,
        PBR = 2
    };
    struct MapAssets
    {
        AssetHandle AlbedoMapHandle = AssetHandle(0);
        AssetHandle NormalMapHandle = AssetHandle(0);
        AssetHandle MetallicMapHandle = AssetHandle(0);
        AssetHandle RoughnessMapHandle = AssetHandle(0);
        AssetHandle AOMapHandle = AssetHandle(0);
    };

    struct MaterialNodeEditorData
    {
        struct NodeData
        {
            int ID = 0;
            int Type = 0; // 0 = PBROutput, 1 = Texture2D
            float PosX = 0.0f;
            float PosY = 0.0f;
            AssetHandle TextureHandle = AssetHandle(0);
        };

        struct LinkData
        {
            int ID = 0;
            int StartPinID = 0;
            int EndPinID = 0;
        };

        std::vector<NodeData> Nodes;
        std::vector<LinkData> Links;
        int NextNodeID = 2;
        int NextLinkID = 1;
    };
    class Material : public Asset
    {
    public:
        Material();

        // Legacy Phong setters
        void setDiffuseColor(const glm::vec4 &color);
        void setAmbientColor(const glm::vec4 &color);
        void setDiffuseTexture(AssetHandle textureHandle);

        // PBR setters
        void setMaterialType(MaterialType type);
        void setName(const std::string &name);
        void setAlbedo(const glm::vec3 &albedo);
        void setMetallic(float metallic);
        void setRoughness(float roughness);
        void setAO(float ao);

        void setAlbedoMap(AssetHandle textureHandle);
        void setNormalMap(AssetHandle textureHandle);
        void setMetallicMap(AssetHandle textureHandle);
        void setRoughnessMap(AssetHandle textureHandle);
        void setAOMap(AssetHandle textureHandle);

        void bind(const std::shared_ptr<Shader> &shader, int slot = 0) const;
        std::shared_ptr<Material> clone() const;
        void copyFrom(const Material &other);

        // Getters
        bool hasTexture() const;
        const glm::vec4 &getDiffuseColor() const;
        const glm::vec4 &getAmbientColor() const;
        AssetHandle getDiffuseTexture() const;
        AssetHandle &getDiffuseTexture();
        MaterialType getType() const;
        const std::string &getName() const;
        const glm::vec3 &getAlbedo() const;
        float getMetallic() const;
        float getRoughness() const;
        float getAO() const;

        AssetHandle getAlbedoMap() const;
        AssetHandle getNormalMap() const;
        AssetHandle getMetallicMap() const;
        AssetHandle getRoughnessMap() const;
        AssetHandle getAOMap() const;

        AssetHandle &getAlbedoMap();
        AssetHandle &getNormalMap();
        AssetHandle &getMetallicMap();
        AssetHandle &getRoughnessMap();
        AssetHandle &getAOMap();
        AssetType getAssetsType() const override;

        const MaterialNodeEditorData &getEditorData() const;
        void setEditorData(const MaterialNodeEditorData &data);

    private:
        void bindPhong(const std::shared_ptr<Shader> &shader, int slot) const;
        void bindPBR(const std::shared_ptr<Shader> &shader, int slot) const;

    private:
        MaterialType Type;
        std::string Name;

        // Phong 材质属性
        glm::vec4 DiffuseColor; // Kd 漫反射
        glm::vec4 AmbientColor; // Ka 环境光
        AssetHandle DiffuseTextureHandle = AssetHandle(0);

        // PBR 材质属性
        glm::vec3 Albedo;
        float Metallic;
        float Roughness;
        float AO;

        MapAssets m_Maps;
        MaterialNodeEditorData m_EditorData;
    };

} // namespace Fermion
