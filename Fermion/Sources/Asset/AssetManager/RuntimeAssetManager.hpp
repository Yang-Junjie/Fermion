#pragma once
#include "AssetManagerBase.hpp"

namespace Fermion {

class RuntimeAssetManager : public AssetManagerBase {
public:
    virtual ~RuntimeAssetManager() override = default;
    using AssetManagerBase::getAsset;
    using AssetManagerBase::init;
    using AssetManagerBase::isAssetLoaded;
    using AssetManagerBase::reloadAsset;
    using AssetManagerBase::shutdown;
    using AssetManagerBase::unloadAsset;
    static AssetHandle importAsset(const std::filesystem::path &) = delete;
};
} // namespace Fermion
