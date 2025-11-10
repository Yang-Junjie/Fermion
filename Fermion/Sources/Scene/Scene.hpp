#pragma once
#include <entt/entt.hpp>
#include "Core/Timestep.hpp"

namespace Fermion
{
    class Scene
    {
    public:
        Scene();
        ~Scene();
        void onUpdate(Timestep ts);

        entt::entity createEntity();

        // temp
        entt::registry &getRegistry() { return m_registry; }

    private:
        entt::registry m_registry;
    };
}