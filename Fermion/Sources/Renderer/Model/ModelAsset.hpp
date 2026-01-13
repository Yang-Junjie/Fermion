#pragma once
#include "Asset/Asset.hpp"

#include <vector>

namespace Fermion
{
    class ModelAsset : public Asset
    {
    public:
        AssetType getAssetsType() const override
        {
            return AssetType::Model;
        }

    public:
        AssetHandle mesh;
        std::vector<AssetHandle> materials;
    };
}
