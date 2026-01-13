#pragma once

#include "AssetImporter.hpp"

namespace Fermion {
class ModelImporter : public AssetImporter {
public:
    AssetMetadata importAsset(const std::filesystem::path &assetPath) override;

private:
    AssetMetadata m_Metadata;
};
} // namespace Fermion
