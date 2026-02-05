#include "fmpch.hpp"
#include "AnimationSampler.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Fermion
{
    template<typename T>
    size_t AnimationSampler::findKeyframeIndex(const std::vector<Keyframe<T>>& keys, float time)
    {
        if (keys.empty())
            return 0;

        // Binary search for the keyframe
        size_t left = 0;
        size_t right = keys.size() - 1;

        while (left < right)
        {
            size_t mid = (left + right + 1) / 2;
            if (keys[mid].time <= time)
                left = mid;
            else
                right = mid - 1;
        }

        return left;
    }

    glm::vec3 AnimationSampler::interpolatePosition(const std::vector<Keyframe<glm::vec3>>& keys, float time)
    {
        if (keys.empty())
            return glm::vec3(0.0f);

        if (keys.size() == 1)
            return keys[0].value;

        size_t index = findKeyframeIndex(keys, time);
        size_t nextIndex = index + 1;

        if (nextIndex >= keys.size())
            return keys[index].value;

        const auto& key0 = keys[index];
        const auto& key1 = keys[nextIndex];

        float dt = key1.time - key0.time;
        float t = (dt > 0.0f) ? (time - key0.time) / dt : 0.0f;
        t = glm::clamp(t, 0.0f, 1.0f);

        return glm::mix(key0.value, key1.value, t);
    }

    glm::quat AnimationSampler::interpolateRotation(const std::vector<Keyframe<glm::quat>>& keys, float time)
    {
        if (keys.empty())
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        if (keys.size() == 1)
            return keys[0].value;

        size_t index = findKeyframeIndex(keys, time);
        size_t nextIndex = index + 1;

        if (nextIndex >= keys.size())
            return keys[index].value;

        const auto& key0 = keys[index];
        const auto& key1 = keys[nextIndex];

        float dt = key1.time - key0.time;
        float t = (dt > 0.0f) ? (time - key0.time) / dt : 0.0f;
        t = glm::clamp(t, 0.0f, 1.0f);

        return glm::slerp(key0.value, key1.value, t);
    }

    glm::vec3 AnimationSampler::interpolateScale(const std::vector<Keyframe<glm::vec3>>& keys, float time)
    {
        if (keys.empty())
            return glm::vec3(1.0f);

        if (keys.size() == 1)
            return keys[0].value;

        size_t index = findKeyframeIndex(keys, time);
        size_t nextIndex = index + 1;

        if (nextIndex >= keys.size())
            return keys[index].value;

        const auto& key0 = keys[index];
        const auto& key1 = keys[nextIndex];

        float dt = key1.time - key0.time;
        float t = (dt > 0.0f) ? (time - key0.time) / dt : 0.0f;
        t = glm::clamp(t, 0.0f, 1.0f);

        return glm::mix(key0.value, key1.value, t);
    }

    BoneTransform AnimationSampler::sampleBone(const BoneChannel& channel, float time, bool looping)
    {
        BoneTransform result;
        result.position = interpolatePosition(channel.positionKeys, time);
        result.rotation = interpolateRotation(channel.rotationKeys, time);
        result.scale = interpolateScale(channel.scaleKeys, time);
        return result;
    }
} // namespace Fermion
