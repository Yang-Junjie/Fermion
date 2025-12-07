#pragma once
#include "AssetTypes.hpp"
#include <unordered_map>
#include <string>

namespace Fermion
{
    inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
        {
            // Scenes
            {".fermion", AssetType::Scene},
            // Textures
            {".png", AssetType::Texture},
            {".jpg", AssetType::Texture},
            {".jpeg", AssetType::Texture},
            // Fonts
            {".ttf", AssetType::Font},
            {".ttc", AssetType::Font},
            {".otf", AssetType::Font},
            // Shaders
            {".glsl", AssetType::Shader},
            {".vert", AssetType::Shader},
            {".frag", AssetType::Shader}};

    inline static AssetType GetAssetTypeFromExtension(const std::string &extension)
    {
        auto it = s_AssetExtensionMap.find(extension);
        return it != s_AssetExtensionMap.end() ? it->second : AssetType::None;
    }
}
