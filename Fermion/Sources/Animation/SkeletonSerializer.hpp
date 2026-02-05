#pragma once
#include "Skeleton.hpp"
#include "Asset/Asset.hpp"
#include <filesystem>
#include <memory>

namespace Fermion
{
    class SkeletonSerializer
    {
    public:
        static bool serialize(const std::filesystem::path &filepath, const Skeleton &skeleton);
        static std::shared_ptr<Skeleton> deserialize(const std::filesystem::path &filepath,
                                                     AssetHandle handle = AssetHandle(0));
    };
} // namespace Fermion
