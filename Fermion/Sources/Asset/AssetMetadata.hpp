#pragma once
#include "fmpch.hpp"
#include "Asset.hpp"


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
} // namespace Fermion
