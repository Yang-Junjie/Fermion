#pragma once
#include "Scene/Scene.hpp"
#include <entt/entt.hpp>
#include "fmpch.hpp"

namespace Fermion
{

    class Entity
    {
    public:
        Entity(entt::entity handle, Scene *scene);

        Entity() = default;
        Entity(const Entity &other) = default;

        explicit operator bool() const { return m_scene != nullptr && m_entityHandle != entt::null; }
        operator entt::entity() const { return m_entityHandle; }
        operator uint32_t() const { return static_cast<uint32_t>(m_entityHandle); }
        bool operator==(const Entity &other) const { return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene; }
        bool operator!=(const Entity &other) const { return !(*this == other); }

        template <typename T>
        bool hasComponent() const
        {
            FMAssert::Assert(m_scene != nullptr, "Entity is null (no owning scene)", __FILE__, __LINE__);
            FMAssert::Assert(m_entityHandle != entt::null, "Entity handle is null", __FILE__, __LINE__);
            return m_scene->m_registry.all_of<T>(m_entityHandle);
        }

        // TODO: 在添加组件的时候执行一些函数操作
        template <typename T, typename... Args>
        T &addComponent(Args &&...args)
        {
            FMAssert::Assert(m_scene != nullptr, "Entity is null (no owning scene)", __FILE__, __LINE__);
            FMAssert::Assert(m_entityHandle != entt::null, "Entity handle is null", __FILE__, __LINE__);
            FMAssert::Assert(!hasComponent<T>(), "Entity already has this component", __FILE__, __LINE__);
            return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);;
        }

      
        template <typename T>
        T &getComponent()
        {
            FMAssert::Assert(m_scene != nullptr, "Entity is null (no owning scene)", __FILE__, __LINE__);
            FMAssert::Assert(m_entityHandle != entt::null, "Entity handle is null", __FILE__, __LINE__);
            FMAssert::Assert(hasComponent<T>(), "Entity does not have this component", __FILE__, __LINE__);
            return m_scene->m_registry.get<T>(m_entityHandle);
        }

        template <typename T>
        void removeComponent()
        {
            FMAssert::Assert(m_scene != nullptr, "Entity is null (no owning scene)", __FILE__, __LINE__);
            FMAssert::Assert(m_entityHandle != entt::null, "Entity handle is null", __FILE__, __LINE__);
            FMAssert::Assert(hasComponent<T>(), "Entity does not have this component", __FILE__, __LINE__);
            m_scene->m_registry.remove<T>(m_entityHandle);
        }

    private:
        entt::entity m_entityHandle{entt::null};
        Scene *m_scene{nullptr};
    };

}
