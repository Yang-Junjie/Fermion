#include "ParticleSystem.hpp"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

#include <random>

class Random
{
public:
    static void Init()
    {
        s_RandomEngine.seed(std::random_device()());
    }

    static float Float()
    {
        return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
    }

private:
    static std::mt19937 s_RandomEngine;
    static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
};

std::mt19937 Random::s_RandomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution;

ParticleSystem::ParticleSystem(uint32_t maxParticle) : m_PoolIndex(maxParticle - 1)
{
    m_ParticlePool.resize(maxParticle);
}

void ParticleSystem::OnUpdate(Fermion::Timestep ts)
{
    for (auto &particle : m_ParticlePool)
    {
        if (!particle.Active)
            continue;

        if (particle.LifeRemaining <= 0.0f)
        {
            particle.Active = false;
            continue;
        }

        particle.LifeRemaining -= ts;
        particle.Position += particle.Velocity * (float)ts;
        particle.Rotation += 0.01f * ts;
    }
}

void ParticleSystem::OnRender(Fermion::OrthographicCamera &camera)
{
    Fermion::Renderer2D::beginScene(camera);
    for (auto &particle : m_ParticlePool)
    {
        if (!particle.Active)
            continue;

        // Fade away particles
        float life = particle.LifeRemaining / particle.LifeTime;
        glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
        // color.a = color.a * life;

        float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

        Fermion::Renderer2D::drawRotatedQuad(particle.Position, glm::vec2(size), particle.Rotation, color);
    }
    Fermion::Renderer2D::endScene();
}

void ParticleSystem::Emit(const ParticleProps &particleProps)
{
    Particle &particle = m_ParticlePool[m_PoolIndex];
    particle.Active = true;
    particle.Position = particleProps.Position;
    particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

    // Velocity
    particle.Velocity = particleProps.Velocity;
    particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
    particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

    // Color
    particle.ColorBegin = particleProps.ColorBegin;
    particle.ColorEnd = particleProps.ColorEnd;

    particle.LifeTime = particleProps.LifeTime;
    particle.LifeRemaining = particleProps.LifeTime;
    particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
    particle.SizeEnd = particleProps.SizeEnd;

    m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
}