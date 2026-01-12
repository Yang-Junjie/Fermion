#pragma once

#include "AssetLoader.hpp"
#include "Renderer/Model/Material.hpp"

namespace Fermion {
class MaterialLoader : public AssetLoader {
public:
    std::shared_ptr<Asset> load(const AssetMetadata &metadata) override;
};
} // namespace Fermion