#include "Asset/Loader/AnimationClipLoader.hpp"
#include "Animation/AnimationClipSerializer.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> AnimationClipLoader::load(const AssetMetadata &metadata)
    {
        auto clip = AnimationClipSerializer::deserialize(metadata.FilePath, metadata.Handle);
        FERMION_ASSERT(clip != nullptr, "Failed to load animation clip!");
        return clip;
    }
} // namespace Fermion
