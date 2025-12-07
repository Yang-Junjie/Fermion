#pragma once
#include "Asset/AssetMetadata.hpp"

#include <memory>

namespace Fermion
{
    class AssetLoader
    {
    public:
        virtual ~AssetLoader() = default;

        virtual std::shared_ptr<Asset> load(const AssetMetadata &metadata) = 0;
    };
}
