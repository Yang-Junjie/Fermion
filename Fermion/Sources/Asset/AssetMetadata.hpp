#pragma once
#include "Asset.hpp"
#include <filesystem>
#include <string>

namespace Fermion
{
    struct AssetMetadata
    {
        AssetHandle Handle;
        AssetType Type;
        std::filesystem::path FilePath;
        std::string Name;

        bool MemoryOnly = false;
    };
}
