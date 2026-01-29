#pragma once
#include <string>
namespace Fermion
{
    enum class AssetType
    {
        None = 0,
        Texture,
        TextureSource, 
        Scene,
        Font,
        Shader,
        Mesh,
        Material,
        Model,
        ModelSource
    };

    namespace AssetUtils
    {
        inline const char *AssetTypeToString(AssetType type)
        {
            switch (type)
            {
            case AssetType::None:
                return "None";
            case AssetType::Texture:
                return "Texture";
            case AssetType::TextureSource:
                return "TextureSource";
            case AssetType::Scene:
                return "Scene";
            case AssetType::Font:
                return "Font";
            case AssetType::Shader:
                return "Shader";
            case AssetType::Mesh:
                return "Mesh";
            case AssetType::Material:
                return "Material";
            case AssetType::Model:
                return "Model";
            case AssetType::ModelSource:
                return "ModelSource";
            }
            return "None";
        }

        inline AssetType StringToAssetType(const std::string &str)
        {
            if (str == "Texture")
                return AssetType::Texture;
            if (str == "TextureSource")
                return AssetType::TextureSource;
            if (str == "Scene")
                return AssetType::Scene;
            if (str == "Font")
                return AssetType::Font;
            if (str == "Shader")
                return AssetType::Shader;
            if (str == "Mesh")
                return AssetType::Mesh;
            if (str == "Material")
                return AssetType::Material;
            if (str == "Model")
                return AssetType::Model;
            if (str == "ModelSource")
                return AssetType::ModelSource;
            return AssetType::None;
        }
    } // namespace AssetUtils
} // namespace Fermion
