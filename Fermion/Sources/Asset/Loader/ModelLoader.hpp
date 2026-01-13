#pragma once

#include "AssetLoader.hpp"

namespace Fermion {
class ModelLoader : public AssetLoader {
public:
    std::shared_ptr<Asset> load(const AssetMetadata &metadata) override;
};
} // namespace Fermion
