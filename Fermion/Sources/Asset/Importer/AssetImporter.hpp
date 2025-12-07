#pragma once
#include "Asset/AssetMetadata.hpp"

#include <filesystem>

namespace Fermion
{
    class AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;
        virtual AssetMetadata importAsset(const std::filesystem::path &assetPath) = 0;
        virtual void writeMetadata(const AssetMetadata &metadata);
        virtual AssetMetadata loadMetadata(const std::filesystem::path &metaFilePath);
    };
}
