#pragma once
#include "fmpch.hpp"
#include "Asset/Asset.hpp"
#include "BoneTransform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Fermion
{
    class Skeleton;

    template<typename T>
    struct Keyframe
    {
        float time = 0.0f;
        T value;

        Keyframe() = default;
        Keyframe(float t, const T& v) : time(t), value(v) {}
    };

    /**
     * @brief Animation data for a single bone
     */
    struct BoneChannel
    {
        std::string boneName;
        int32_t boneIndex = -1;  // Resolved after binding to skeleton

        std::vector<Keyframe<glm::vec3>> positionKeys;
        std::vector<Keyframe<glm::quat>> rotationKeys;
        std::vector<Keyframe<glm::vec3>> scaleKeys;
    };

    /**
     * @brief An animation clip containing keyframe data for multiple bones
     */
    class AnimationClip : public Asset
    {
    public:
        AnimationClip() = default;
        AnimationClip(const std::string& name, float duration, float ticksPerSecond = 25.0f);
        ~AnimationClip() override = default;

        AssetType getAssetsType() const override { return AssetType::AnimationClip; }

        const std::string& getName() const { return m_name; }
        void setName(const std::string& name) { m_name = name; }

        float getDuration() const { return m_duration; }
        void setDuration(float duration) { m_duration = duration; }

        float getTicksPerSecond() const { return m_ticksPerSecond; }
        void setTicksPerSecond(float tps) { m_ticksPerSecond = tps; }

        /**
         * @brief Add a bone channel to the animation
         */
        void addChannel(BoneChannel channel);

        /**
         * @brief Get all bone channels
         */
        const std::vector<BoneChannel>& getChannels() const { return m_channels; }
        std::vector<BoneChannel>& getChannels() { return m_channels; }

        /**
         * @brief Find a channel by bone name
         * @return Pointer to channel, or nullptr if not found
         */
        const BoneChannel* findChannel(const std::string& boneName) const;
        BoneChannel* findChannel(const std::string& boneName);

        /**
         * @brief Bind this animation to a skeleton, resolving bone names to indices
         * @param skeleton The skeleton to bind to
         */
        void bindSkeleton(const std::shared_ptr<Skeleton>& skeleton);

    private:
        std::string m_name;
        float m_duration = 0.0f;
        float m_ticksPerSecond = 25.0f;
        std::vector<BoneChannel> m_channels;
        std::unordered_map<std::string, size_t> m_channelNameMap;
    };
} // namespace Fermion
