#pragma once

#include "AssetManagerBase.hpp"

namespace Fermion
{
    class EditorAssetManager : public AssetManagerBase
    {
    public:
        virtual ~EditorAssetManager() override = default;
        using AssetManagerBase::getAsset;
        using AssetManagerBase::importAsset;
        using AssetManagerBase::init;
        using AssetManagerBase::isAssetLoaded;
        using AssetManagerBase::reloadAsset;
        using AssetManagerBase::shutdown;
        using AssetManagerBase::unloadAsset;
    };
}
