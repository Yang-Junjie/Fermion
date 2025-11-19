#pragma once

#include <unordered_map>

#include "AssetTypes.hpp"

namespace Fermion
{

    inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
        {

            // Fermion
            {".fermion", AssetType::Scene},

            // Textures
            {".png", AssetType::Texture},
            {".jpg", AssetType::Texture},
            {".jpeg", AssetType::Texture},

            // Fonts
            {".ttf", AssetType::Font},
            {".ttc", AssetType::Font},
            {".otf", AssetType::Font},

    };

}
