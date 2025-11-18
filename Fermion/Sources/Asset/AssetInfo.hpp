#pragma once
#include "Asset.hpp"

#include <filesystem>
namespace Fermion
{

    struct AssetInfo
    {
        AssetHandle Handle ;
        AssetType Type;
        std::filesystem::path FilePath;
    };
}