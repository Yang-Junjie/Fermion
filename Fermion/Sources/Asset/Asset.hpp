#pragma once
#include "Core/UUID.hpp"
#include "AssetTypes.hpp"

namespace Fermion
{
    using AssetHandle = UUID;

    class Asset
    {
    public:
        AssetHandle handle;

        virtual ~Asset() = default;
        virtual AssetType getAssetsType() const { return AssetType::None; }

        virtual bool operator==(const Asset& other) const { return handle == other.handle; }
        virtual bool operator!=(const Asset& other) const { return !(*this == other); }
    };
}
