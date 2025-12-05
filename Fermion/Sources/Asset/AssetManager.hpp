#pragma once
#include "fmpch.hpp"
#include "Asset.hpp"
#include "AssetMetadata.hpp"
#include "AssetRegistry.hpp"
#include "AssetExtensions.hpp"

#include <filesystem>

namespace Fermion
{
    class AssetManager
    {
    public:
        static void init(const std::filesystem::path& assetDirectory);
        static void shutdown();

        template <typename T>
        static std::shared_ptr<T> getAsset(AssetHandle handle)
        {

            auto it = s_loadedAssets.find(handle);
            if (it != s_loadedAssets.end())
                return std::dynamic_pointer_cast<T>(it->second);

            if (!AssetRegistry::exists(handle))
                return nullptr;

            std::shared_ptr<Asset> asset = loadAssetInternal(handle);
            if (!asset)
                return nullptr;

            s_loadedAssets[handle] = asset;
            return std::dynamic_pointer_cast<T>(asset);
        }

        static bool isAssetLoaded(AssetHandle handle);
        static void reloadAsset(AssetHandle handle);
        static void unloadAsset(AssetHandle handle);

        static AssetHandle importAsset(const std::filesystem::path& path); 

    private:
        static std::shared_ptr<Asset> loadAssetInternal(AssetHandle handle);
        static std::unordered_map<AssetHandle, std::shared_ptr<Asset>> s_loadedAssets;
        static std::filesystem::path s_assetDirectory;
    };
}
