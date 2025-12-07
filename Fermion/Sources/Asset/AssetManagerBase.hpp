#pragma once

#include "AssetManager.hpp"

namespace Fermion
{
    class AssetManagerBase
    {
    public:
        static void init(const std::filesystem::path &assetDirectory)
        {
            AssetManager::init(assetDirectory);
        }

        static void shutdown()
        {
            AssetManager::shutdown();
        }

        template <typename T>
        static std::shared_ptr<T> getAsset(AssetHandle handle)
        {
            return AssetManager::getAsset<T>(handle);
        }

        static bool isAssetLoaded(AssetHandle handle)
        {
            return AssetManager::isAssetLoaded(handle);
        }

        static void reloadAsset(AssetHandle handle)
        {
            AssetManager::reloadAsset(handle);
        }

        static void unloadAsset(AssetHandle handle)
        {
            AssetManager::unloadAsset(handle);
        }

        static AssetHandle importAsset(const std::filesystem::path &path)
        {
            return AssetManager::importAsset(path);
        }
    };
}

