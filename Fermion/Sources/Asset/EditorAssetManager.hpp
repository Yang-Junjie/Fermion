#pragma once

#include "AssetManagerBase.hpp"

namespace Fermion
{
    class EditorAssetManager : public AssetManagerBase
    {
    public:
        using AssetManagerBase::init;
        using AssetManagerBase::shutdown;
        using AssetManagerBase::getAsset;
        using AssetManagerBase::isAssetLoaded;
        using AssetManagerBase::reloadAsset;
        using AssetManagerBase::unloadAsset;
        using AssetManagerBase::importAsset;
    };
}
