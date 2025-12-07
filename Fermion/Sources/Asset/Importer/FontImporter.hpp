#pragma once

#include "AssetImporter.hpp"

namespace Fermion
{
    class FontImporter : public AssetImporter
    {
    public:
        AssetMetadata importAsset(const std::filesystem::path &assetPath) override;
    };
}

