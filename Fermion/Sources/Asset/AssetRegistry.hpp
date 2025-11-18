#pragma once
#include "Asset.hpp"
#include "AssetInfo.hpp"
#include <unordered_map>
namespace Fermion
{
    class AssetRegistry
    {
    public:
        static AssetInfo &get(const AssetHandle handle)
        {
            return m_assetsRegistry.at(handle);
        }
        static void set(const AssetHandle handle, const AssetInfo &info)
        {
            m_assetsRegistry[handle] = info;
        }
        static void shutdown()
        {
            m_assetsRegistry.clear();
        }

    private:
        static std::unordered_map<AssetHandle, AssetInfo> m_assetsRegistry;
    };
}