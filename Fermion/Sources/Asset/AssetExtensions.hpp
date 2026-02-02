#pragma once
#include "AssetTypes.hpp"
#include <unordered_map>
#include <string>

namespace Fermion
{
    inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
        {
            // Scenes
            {".fmscene", AssetType::Scene},
            {".ftex", AssetType::Texture},

            // Textures
            {".png", AssetType::TextureSource},
            {".jpg", AssetType::TextureSource},
            {".jpeg", AssetType::TextureSource},
            {".hdr", AssetType::TextureSource},
            // Fonts
            {".ttf", AssetType::Font},
            {".ttc", AssetType::Font},
            {".otf", AssetType::Font},
            // Shaders
            {".glsl", AssetType::Shader},
            {".vert", AssetType::Shader},
            {".frag", AssetType::Shader},

            // model source
            {".obj", AssetType::ModelSource},
            {".glb", AssetType::ModelSource},
            {".gltf", AssetType::ModelSource},

            // mesh
            {".fmesh", AssetType::Mesh},
            // model
            {".fmodel", AssetType::Model},
            // Material
            {".fmat", AssetType::Material},
    };

    inline static AssetType GetAssetTypeFromExtension(const std::string &extension)
    {
        auto it = s_AssetExtensionMap.find(extension);
        return it != s_AssetExtensionMap.end() ? it->second : AssetType::None;
    }

    inline static std::string GetAssetExtensionFromType(AssetType type)
    {
        for (auto &[ext, t] : s_AssetExtensionMap)
        {
            if (t == type)
                return ext;
        }
        return "";
    }
} // namespace Fermion
