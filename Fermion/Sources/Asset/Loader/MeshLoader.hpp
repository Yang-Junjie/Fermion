#pragma once

#include "Renderer/Mesh.hpp"
#include "AssetLoader.hpp"

namespace Fermion
{
    class MeshLoader : public AssetLoader
    {
    public:
        std::shared_ptr<Asset> load(const AssetMetadata &metadata) override;
    };
}
