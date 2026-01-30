#pragma once

#include "../AssetManager.hpp"

// 现在EditorAssetManager和RuntimeAssetManager的实现没有任何差异，后期划分职责
namespace Fermion {
class AssetManagerBase {
public:
    AssetManagerBase() = default;
    virtual ~AssetManagerBase() = default;

    void init(const std::filesystem::path &assetDirectory) {
        AssetManager::init(assetDirectory);
    }

    void shutdown() {
        AssetManager::shutdown();
    }

    template <typename T>
    std::shared_ptr<T> getAsset(AssetHandle handle) {
        return AssetManager::getAsset<T>(handle);
    }

    bool isAssetLoaded(AssetHandle handle) {
        return AssetManager::isAssetLoaded(handle);
    }

    void reloadAsset(AssetHandle handle) {
        AssetManager::reloadAsset(handle);
    }

    void unloadAsset(AssetHandle handle) {
        AssetManager::unloadAsset(handle);
    }

    AssetHandle importAsset(const std::filesystem::path &path) {
        return AssetManager::importAsset(path);
    }

    template <typename T>
    std::shared_ptr<T> getDefaultAssetForType() {
        return AssetManager::getDefaultAssetForType<T>();
    }
};
} // namespace Fermion
