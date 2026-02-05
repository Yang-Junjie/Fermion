#pragma once

#include "Animation/AnimationClip.hpp"
#include "AssetLoader.hpp"

namespace Fermion {
class AnimationClipLoader : public AssetLoader {
public:
    std::shared_ptr<Asset> load(const AssetMetadata &metadata) override;
};
} // namespace Fermion
