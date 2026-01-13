#pragma once

#include "ModelAsset.hpp"

#include "Asset/AssetMetadata.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace Fermion {

class ModelSerializer {
public:
    static AssetMetadata serialize(const std::filesystem::path &modelPath, AssetHandle meshHandle,
                                   const std::vector<AssetHandle> &materials);

    static std::shared_ptr<ModelAsset> deserialize(const std::filesystem::path &modelPath,
                                                   AssetHandle handle = AssetHandle(0));
};

} // namespace Fermion
