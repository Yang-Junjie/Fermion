#pragma once

#include <memory>
#include <unordered_map>
#include "Core/Timestep.hpp"
#include "Core/UUID.hpp"

namespace JPH
{
    class PhysicsSystem;
    class TempAllocator;
    class JobSystem;
}

namespace Fermion
{
    class Scene;
    class Entity;

    class Physics3DWorld
    {
    public:
        Physics3DWorld();
        ~Physics3DWorld();

        void start(Scene *scene);
        void stop(Scene *scene);
        void step(Scene *scene, Timestep ts);
        void addBody(Scene *scene, Entity entity);

        bool isActive() const;
        JPH::PhysicsSystem *getPhysicsSystem() const { return m_physicsSystem.get(); }

    private:
        void ensureInitialized();

        std::unique_ptr<JPH::PhysicsSystem> m_physicsSystem;
        std::unique_ptr<JPH::TempAllocator> m_tempAllocator;
        std::unique_ptr<JPH::JobSystem> m_jobSystem;
        std::unordered_map<UUID, uint32_t> m_bodyMap;
    };
} // namespace Fermion
