#include "Asset/Loader/SkeletonLoader.hpp"
#include "Animation/SkeletonSerializer.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> SkeletonLoader::load(const AssetMetadata &metadata)
    {
        auto skeleton = SkeletonSerializer::deserialize(metadata.FilePath, metadata.Handle);
        FERMION_ASSERT(skeleton != nullptr, "Failed to load skeleton!");
        return skeleton;
    }
} // namespace Fermion
