#pragma once

#include "AssetImporter.hpp"

namespace Fermion {
class SceneImporter : public AssetImporter {
public:
    AssetMetadata importAsset(const std::filesystem::path &assetPath) override;
};
} // namespace Fermion
