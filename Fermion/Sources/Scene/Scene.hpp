#pragma once
#include <entt/entt.hpp>
#include "Core/Timestep.hpp"

namespace Fermion
{
    class Entity;
    class Scene
    {
    public:
        Scene();
        ~Scene();
        void onUpdate(Timestep ts);

        Entity createEntity(std::string name = std::string());

    private:
        entt::registry m_registry;

        friend class Entity;
    };
}