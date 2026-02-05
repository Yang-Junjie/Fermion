#pragma once
#include "fmpch.hpp"
#include "AnimationClip.hpp"
#include "BoneTransform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Fermion
{
    /**
     * @brief Utility class for sampling animation keyframes
     */
    class AnimationSampler
    {
    public:
        /**
         * @brief Sample a bone channel at a given time
         * @param channel The bone channel to sample
         * @param time The time to sample at
         * @param looping Whether the animation should loop
         * @return The interpolated bone transform
         */
        static BoneTransform sampleBone(const BoneChannel& channel, float time, bool looping = true);

    private:
        /**
         * @brief Find the index of the keyframe before the given time
         */
        template<typename T>
        static size_t findKeyframeIndex(const std::vector<Keyframe<T>>& keys, float time);

        /**
         * @brief Interpolate position keyframes
         */
        static glm::vec3 interpolatePosition(const std::vector<Keyframe<glm::vec3>>& keys, float time);

        /**
         * @brief Interpolate rotation keyframes
         */
        static glm::quat interpolateRotation(const std::vector<Keyframe<glm::quat>>& keys, float time);

        /**
         * @brief Interpolate scale keyframes
         */
        static glm::vec3 interpolateScale(const std::vector<Keyframe<glm::vec3>>& keys, float time);
    };
} // namespace Fermion
