#pragma once

namespace Fermion
{
    enum class AssetType
    {
        None = 0,
        Texture,
        Scene,
        Font,
        Shader
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
            case AssetType::Scene:
                return "Scene";
            case AssetType::Font:
                return "Font";
            case AssetType::Shader:
                return "Shader";
            }
            return "None";
        }

        inline AssetType StringToAssetType(const std::string &str)
        {
            if (str == "Texture")
                return AssetType::Texture;
            if (str == "Scene")
                return AssetType::Scene;
            if (str == "Font")
                return AssetType::Font;
            if (str == "Shader")
                return AssetType::Shader;
            return AssetType::None;
        }
    }
}
