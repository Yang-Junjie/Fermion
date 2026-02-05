#pragma once
#include "AnimationClip.hpp"
#include "Asset/Asset.hpp"
#include <filesystem>
#include <memory>

namespace Fermion
{
    class AnimationClipSerializer
    {
    public:
        static bool serialize(const std::filesystem::path &filepath, const AnimationClip &clip);
        static std::shared_ptr<AnimationClip> deserialize(const std::filesystem::path &filepath,
                                                          AssetHandle handle = AssetHandle(0));
    };
} // namespace Fermion
