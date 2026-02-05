#pragma once
#include "fmpch.hpp"
#include "Skeleton.hpp"
#include "AnimationClip.hpp"
#include "BoneTransform.hpp"
#include "Core/Timestep.hpp"
#include <glm/glm.hpp>

namespace Fermion
{

    class Animator
    {
    public:
        Animator() = default;
        ~Animator() = default;

        void setSkeleton(const std::shared_ptr<Skeleton>& skeleton);   
        std::shared_ptr<Skeleton> getSkeleton() const { return m_skeleton; }

        void play(const std::shared_ptr<AnimationClip>& clip);
        void pause() { m_playing = false; }
        void resume() { m_playing = true; }
        void stop();
        bool isPlaying() const { return m_playing; }
        void setSpeed(float speed) { m_speed = speed; }
        float getSpeed() const { return m_speed; }

        void setLooping(bool loop) { m_looping = loop; }
        bool isLooping() const { return m_looping; }
        float getCurrentTime() const { return m_currentTime; }
        void setCurrentTime(float time) { m_currentTime = time; }


        void update(Timestep ts);


        const std::vector<glm::mat4>& getFinalBoneMatrices() const { return m_finalBoneMatrices; }

        std::shared_ptr<AnimationClip> getCurrentClip() const { return m_currentClip; }

    private:

        void computeBoneMatrices();
        void sampleAnimation();

    private:
        std::shared_ptr<Skeleton> m_skeleton;
        std::shared_ptr<AnimationClip> m_currentClip;

        float m_currentTime = 0.0f;
        float m_speed = 1.0f;
        bool m_playing = false;
        bool m_looping = true;

        std::vector<BoneTransform> m_localPoses;
        std::vector<glm::mat4> m_finalBoneMatrices;
    };
} // namespace Fermion
