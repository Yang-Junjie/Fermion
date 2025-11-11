#pragma once
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
namespace Fermion
{
    class ScriptableEntity
    {
    public:
        template <typename T>
        T &getComponent()
        {
            return m_entity.getComponent<T>();
        }

    private:
        Entity m_entity;
        friend class Scene;
    };
}