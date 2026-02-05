#include "fmpch.hpp"
#include "Skeleton.hpp"
#include "Core/Log.hpp"
#include <format>

namespace Fermion
{
    int32_t Skeleton::addBone(const std::string& name, int32_t parentIndex,
                              const glm::mat4& offsetMatrix, const BoneTransform& localBindPose)
    {
        if (m_bones.size() >= MAX_BONES)
        {
            Log::Warn(std::format("Skeleton::addBone: Maximum bone count ({}) exceeded", MAX_BONES));
            return -1;
        }

        if (m_boneNameMap.find(name) != m_boneNameMap.end())
        {
            Log::Warn(std::format("Skeleton::addBone: Bone '{}' already exists", name));
            return m_boneNameMap[name];
        }

        int32_t index = static_cast<int32_t>(m_bones.size());

        BoneInfo bone;
        bone.name = name;
        bone.parentIndex = parentIndex;
        bone.offsetMatrix = offsetMatrix;
        bone.localBindPose = localBindPose;

        m_bones.push_back(bone);
        m_boneNameMap[name] = index;

        return index;
    }

    int32_t Skeleton::findBoneIndex(const std::string& name) const
    {
        auto it = m_boneNameMap.find(name);
        if (it != m_boneNameMap.end())
            return it->second;
        return -1;
    }
} // namespace Fermion
