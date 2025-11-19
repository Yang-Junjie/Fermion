#include "AssetRegistry.hpp"

namespace Fermion
{
    std::unordered_map<AssetHandle, AssetInfo> AssetRegistry::s_assetsRegistry;

    AssetInfo& AssetRegistry::get(AssetHandle handle)
    {
        return s_assetsRegistry.at(handle);
    }

    void AssetRegistry::set(AssetHandle handle, const AssetInfo& info)
    {
        s_assetsRegistry[handle] = info;
    }

    bool AssetRegistry::exists(AssetHandle handle)
    {
        return s_assetsRegistry.find(handle) != s_assetsRegistry.end();
    }

    size_t AssetRegistry::remove(AssetHandle handle)
    {
        return s_assetsRegistry.erase(handle);
    }

    void AssetRegistry::clear()
    {
        s_assetsRegistry.clear();
    }

    const std::unordered_map<AssetHandle, AssetInfo>& AssetRegistry::getRegistry()
    {
        return s_assetsRegistry;
    }
}
