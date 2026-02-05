#pragma once

#include "Animation/Skeleton.hpp"
#include "AssetLoader.hpp"

namespace Fermion {
class SkeletonLoader : public AssetLoader {
public:
    std::shared_ptr<Asset> load(const AssetMetadata &metadata) override;
};
} // namespace Fermion
