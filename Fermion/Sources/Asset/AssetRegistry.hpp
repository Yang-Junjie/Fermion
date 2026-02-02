#pragma once
#include "fmpch.hpp"
#include "AssetMetadata.hpp"

namespace Fermion
{
    class AssetRegistry
    {
    public:
        static AssetMetadata &get(AssetHandle handle);
        static void set(AssetHandle handle, const AssetMetadata &info);
        static bool exists(AssetHandle handle);
        static size_t remove(AssetHandle handle);
        static void clear();
        static const std::unordered_map<AssetHandle, AssetMetadata> &getRegistry();

    private:
        static std::unordered_map<AssetHandle, AssetMetadata> s_assetsRegistry;
    };
} // namespace Fermion
