#pragma once
#include "fmpch.hpp"
#include "Asset.hpp"
#include "AssetRegistry.hpp"
#include "Loader/AssetLoader.hpp"

#include <unordered_map>
#include <filesystem>
#include <memory>

namespace Fermion
{
class AssetManager {
public:
    static void init(const std::filesystem::path &assetDirectory);
    static void shutdown();

    template <typename T>
    static std::shared_ptr<T> getAsset(AssetHandle handle) {
        auto it = s_loadedAssets.find(handle);
        if (it != s_loadedAssets.end())
            return std::dynamic_pointer_cast<T>(it->second);

        if (!AssetRegistry::exists(handle))
            return nullptr;

        auto asset = loadAssetInternal(handle);
        if (!asset)
            return nullptr;

        s_loadedAssets[handle] = asset;
        return std::dynamic_pointer_cast<T>(asset);
    }

    static bool isAssetLoaded(AssetHandle handle);
    static void reloadAsset(AssetHandle handle);
    static void unloadAsset(AssetHandle handle);
    static AssetHandle importAsset(const std::filesystem::path &path);
    static AssetHandle addMemoryOnlyAsset(std::shared_ptr<Asset> asset);

    static std::shared_ptr<Asset> getAssetMetadata(AssetHandle handle);

private:
    struct AssetTypeHash {
        std::size_t operator()(AssetType type) const noexcept {
            return static_cast<std::size_t>(type);
        }
    };

    static void ensureDefaultLoaders();

    static std::shared_ptr<Asset> loadAssetInternal(AssetHandle handle);
    static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_loadedAssets;
    // static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_MemoryOnly;
    static std::unordered_map<AssetType, std::unique_ptr<AssetLoader>, AssetTypeHash> s_assetLoaders;
    static std::filesystem::path s_assetDirectory;
};
} // namespace Fermion
