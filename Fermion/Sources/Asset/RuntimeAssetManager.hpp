#pragma once

#include "AssetManagerBase.hpp"

namespace Fermion
{

    class RuntimeAssetManager : public AssetManagerBase
    {
    public:
        using AssetManagerBase::init;
        using AssetManagerBase::shutdown;
        using AssetManagerBase::getAsset;
        using AssetManagerBase::isAssetLoaded;
        using AssetManagerBase::reloadAsset;
        using AssetManagerBase::unloadAsset;
        static AssetHandle importAsset(const std::filesystem::path &) = delete;
    };
}
