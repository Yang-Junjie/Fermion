#include "fmpch.hpp"
#include "AnimationClip.hpp"
#include "Skeleton.hpp"

namespace Fermion
{
    AnimationClip::AnimationClip(const std::string& name, float duration, float ticksPerSecond)
        : m_name(name), m_duration(duration), m_ticksPerSecond(ticksPerSecond)
    {
    }

    void AnimationClip::addChannel(BoneChannel channel)
    {
        m_channelNameMap[channel.boneName] = m_channels.size();
        m_channels.push_back(std::move(channel));
    }

    const BoneChannel* AnimationClip::findChannel(const std::string& boneName) const
    {
        auto it = m_channelNameMap.find(boneName);
        if (it != m_channelNameMap.end())
            return &m_channels[it->second];
        return nullptr;
    }

    BoneChannel* AnimationClip::findChannel(const std::string& boneName)
    {
        auto it = m_channelNameMap.find(boneName);
        if (it != m_channelNameMap.end())
            return &m_channels[it->second];
        return nullptr;
    }

    void AnimationClip::bindSkeleton(const std::shared_ptr<Skeleton>& skeleton)
    {
        if (!skeleton)
            return;

        for (auto& channel : m_channels)
        {
            channel.boneIndex = skeleton->findBoneIndex(channel.boneName);
        }
    }
} // namespace Fermion
