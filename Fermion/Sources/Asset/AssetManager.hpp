#pragma once
#include "Asset.hpp"
#include "AssetInfo.hpp"
#include "AssetRegistry.hpp"

namespace Fermion
{
    class AssetManager
    {
    public:
        static void shutdown();
        static void loadAsset(const AssetHandle &info);
    };
}