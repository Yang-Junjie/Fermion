#include "fmpch.hpp"
#include "Animator.hpp"
#include "AnimationSampler.hpp"

namespace Fermion
{
    void Animator::setSkeleton(const std::shared_ptr<Skeleton>& skeleton)
    {
        m_skeleton = skeleton;
        if (m_skeleton)
        {
            size_t boneCount = m_skeleton->getBoneCount();
            m_localPoses.resize(boneCount);
            m_finalBoneMatrices.resize(boneCount, glm::mat4(1.0f));

            // Initialize with bind pose
            for (size_t i = 0; i < boneCount; ++i)
            {
                m_localPoses[i] = m_skeleton->getBone(i).localBindPose;
            }

            // Compute initial bone matrices for bind pose
            computeBoneMatrices();
        }
    }

    void Animator::play(const std::shared_ptr<AnimationClip>& clip)
    {
        m_currentClip = clip;
        m_currentTime = 0.0f;
        m_playing = true;

        if (m_currentClip && m_skeleton)
        {
            m_currentClip->bindSkeleton(m_skeleton);
        }
    }

    void Animator::stop()
    {
        m_playing = false;
        m_currentTime = 0.0f;
    }

    void Animator::update(Timestep ts)
    {
        if (!m_skeleton || !m_currentClip || !m_playing)
        {
            // Even if not playing, ensure matrices are computed for bind pose
            if (m_skeleton && m_finalBoneMatrices.empty())
            {
                computeBoneMatrices();
            }
            return;
        }

        // Advance time
        float duration = m_currentClip->getDuration();
        m_currentTime += ts.getSeconds() * m_speed * m_currentClip->getTicksPerSecond();

        if (m_looping && duration > 0.0f)
        {
            m_currentTime = std::fmod(m_currentTime, duration);
            if (m_currentTime < 0.0f)
                m_currentTime += duration;
        }
        else if (m_currentTime >= duration)
        {
            m_currentTime = duration;
            m_playing = false;
        }
        else if (m_currentTime < 0.0f)
        {
            m_currentTime = 0.0f;
            m_playing = false;
        }

        // Sample animation at current time
        sampleAnimation();

        // Compute final bone matrices
        computeBoneMatrices();
    }

    void Animator::sampleAnimation()
    {
        if (!m_currentClip || !m_skeleton)
            return;

        // Start with bind pose for all bones
        for (size_t i = 0; i < m_skeleton->getBoneCount(); ++i)
        {
            m_localPoses[i] = m_skeleton->getBone(i).localBindPose;
        }

        // Sample each channel
        for (const auto& channel : m_currentClip->getChannels())
        {
            if (channel.boneIndex >= 0 && channel.boneIndex < static_cast<int32_t>(m_localPoses.size()))
            {
                m_localPoses[channel.boneIndex] = AnimationSampler::sampleBone(channel, m_currentTime, m_looping);
            }
        }
    }

    void Animator::computeBoneMatrices()
    {
        if (!m_skeleton)
            return;

        size_t boneCount = m_skeleton->getBoneCount();
        std::vector<glm::mat4> globalTransforms(boneCount);

        // Compute global transforms by traversing parent->child
        for (size_t i = 0; i < boneCount; ++i)
        {
            const auto& bone = m_skeleton->getBone(i);
            glm::mat4 localMatrix = m_localPoses[i].toMatrix();

            if (bone.parentIndex >= 0 && bone.parentIndex < static_cast<int32_t>(boneCount))
            {
                globalTransforms[i] = globalTransforms[bone.parentIndex] * localMatrix;
            }
            else
            {
                // Root bone
                globalTransforms[i] = localMatrix;
            }

            // Final matrix = global transform * offset matrix (inverse bind pose)
            m_finalBoneMatrices[i] = globalTransforms[i] * bone.offsetMatrix;
        }
    }
} // namespace Fermion
