#pragma once
#include "Asset.hpp"
#include "AssetInfo.hpp"
#include <unordered_map>

namespace Fermion
{
    class AssetRegistry
    {
    public:
        static AssetInfo& get(AssetHandle handle);
        static void set(AssetHandle handle, const AssetInfo& info);
        static bool exists(AssetHandle handle);
        static size_t remove(AssetHandle handle);
        static void clear();
        static const std::unordered_map<AssetHandle, AssetInfo>& getRegistry();

    private:
        static std::unordered_map<AssetHandle, AssetInfo> s_assetsRegistry;
    };
}
