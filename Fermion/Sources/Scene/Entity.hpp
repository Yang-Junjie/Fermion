#pragma once
#include "Scene/Scene.hpp"
#include "fmpch.hpp"
#include <type_traits>
#include "Core/UUID.hpp"
#include "Components.hpp"

namespace Fermion
{
    // Forward declaration for compile-time checks in addComponent
    struct CameraComponent;

    class Entity
    {
    public:
        Entity(entt::entity handle, Scene *scene);

        Entity() = default;

        Entity(const Entity &other) = default;

        [[nodiscard]] bool isValid() const
        {
            return m_scene != nullptr && m_entityHandle != entt::null && m_scene->getRegistry().valid(m_entityHandle);
        }

        explicit operator bool() const
        {
            return isValid();
        }

        operator entt::entity() const
        {
            return m_entityHandle;
        }

        operator uint32_t() const
        {
            return static_cast<uint32_t>(m_entityHandle);
        }

        bool operator==(const Entity &other) const
        {
            return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
        }

        bool operator!=(const Entity &other) const
        {
            return !(*this == other);
        }

        template <typename T>
        bool hasComponent() const
        {
            FERMION_ASSERT(m_scene != nullptr, "Scene is null (no owning scene)");
            FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
            FERMION_ASSERT(m_scene->getRegistry().valid(m_entityHandle),
                           "Entity handle is invalid (entity was probably destroyed)");
            return m_scene->getRegistry().all_of<T>(m_entityHandle);
        }

        template <typename T, typename... Args>
        T &addComponent(Args &&...args)
        {
            FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
            FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
            FERMION_ASSERT(m_scene->getRegistry().valid(m_entityHandle),
                           "Entity handle is invalid (entity was probably destroyed)");
            FERMION_ASSERT(!hasComponent<T>(), "Entity already has this component");

            T &component = m_scene->getRegistry().emplace<T>(m_entityHandle, std::forward<Args>(args)...);

            if constexpr (std::is_same_v<T, CameraComponent>)
            {
                const uint32_t vw = m_scene->getViewportWidth();
                const uint32_t vh = m_scene->getViewportHeight();
                if (vw > 0 && vh > 0)
                {
                    component.camera.setViewportSize(vw, vh);
                }
            }

            return component;
        }

        template <typename T>
        T &getComponent()
        {
            FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
            FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
            FERMION_ASSERT(m_scene->getRegistry().valid(m_entityHandle),
                           "Entity handle is invalid (entity was probably destroyed)");
            FERMION_ASSERT(hasComponent<T>(), "Entity does not have this component");
            return m_scene->getRegistry().get<T>(m_entityHandle);
        }
        template <typename T>
        const T &getComponent() const
        {
            FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
            FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
            FERMION_ASSERT(m_scene->getRegistry().valid(m_entityHandle),
                           "Entity handle is invalid (entity was probably destroyed)");
            FERMION_ASSERT(hasComponent<T>(), "Entity does not have this component");
            return m_scene->getRegistry().get<T>(m_entityHandle);
        }

        template <typename T>
        void removeComponent()
        {
            FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
            FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
            FERMION_ASSERT(m_scene->getRegistry().valid(m_entityHandle),
                           "Entity handle is invalid (entity was probably destroyed)");
            FERMION_ASSERT(hasComponent<T>(), "Entity does not have this component");
            m_scene->getRegistry().remove<T>(m_entityHandle);
        }

        template <typename T, typename... Args>
        T &addOrReplaceComponent(Args &&...args)
        {
            FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
            FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
            FERMION_ASSERT(m_scene->getRegistry().valid(m_entityHandle),
                           "Entity handle is invalid (entity was probably destroyed)");
            T &component = m_scene->getRegistry().emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
            if constexpr (std::is_same_v<T, CameraComponent>)
            {
                const uint32_t vw = m_scene->getViewportWidth();
                const uint32_t vh = m_scene->getViewportHeight();
                if (vw > 0 && vh > 0)
                {
                    component.camera.setViewportSize(vw, vh);
                }
            }
            return component;
        }

        UUID getUUID()
        {
            return getComponent<IDComponent>().ID;
        }

        std::string getName()
        {
            return getComponent<TagComponent>().tag;
        }

        Entity getParent();
        void setParent(Entity parent)
        {
            Entity currentParent = getParent();
            if (currentParent == parent)
                return;

            // If changing parent, remove child from existing parent
            if (currentParent)
                currentParent.removeChild(*this);

            // Setting to null is okay
            setParentUUID(parent ? parent.getUUID() : UUID(0));

            if (parent)
            {
                auto &parentChildren = parent.getChildren();
                UUID uuid = getUUID();
                if (std::find(parentChildren.begin(), parentChildren.end(), uuid) == parentChildren.end())
                    parentChildren.emplace_back(getUUID());
            }
        }
        void setParentUUID(UUID parent) { getComponent<RelationshipComponent>().parentHandle = parent; }
        UUID getParentUUID() const { return getComponent<RelationshipComponent>().parentHandle; }

        std::vector<UUID> &getChildren() { return getComponent<RelationshipComponent>().children; }
        const std::vector<UUID> &getChildren() const { return getComponent<RelationshipComponent>().children; }

        bool removeChild(Entity child)
        {
            UUID childId = child.getUUID();
            std::vector<UUID> &children = getChildren();
            auto it = std::find(children.begin(), children.end(), childId);
            if (it != children.end())
            {
                children.erase(it);
                return true;
            }

            return false;
        }
        TransformComponent &Transform() { return getComponent<TransformComponent>(); }
        const glm::mat4 &Transform() const { return getComponent<TransformComponent>().getTransform(); }

    private:
        entt::entity m_entityHandle{entt::null};
        Scene *m_scene{nullptr};
    };
} // namespace Fermion
