#pragma once
#include "Scene/Scene.hpp"
#include <entt/entt.hpp>
#include "fmpch.hpp"
#include <type_traits>
#include "Core/UUID.hpp"
#include "Components.hpp"

namespace Fermion {
// Forward declaration for compile-time checks in addComponent
struct CameraComponent;

class Entity {
public:
    Entity(entt::entity handle, Scene *scene);

    Entity() = default;
    Entity(const Entity &other) = default;

    explicit operator bool() const {
        return m_scene != nullptr && m_entityHandle != entt::null;
    }
    operator entt::entity() const {
        return m_entityHandle;
    }
    operator uint32_t() const {
        return static_cast<uint32_t>(m_entityHandle);
    }
    bool operator==(const Entity &other) const {
        return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
    }
    bool operator!=(const Entity &other) const {
        return !(*this == other);
    }

    template <typename T>
    bool hasComponent() const {
        FERMION_ASSERT(m_scene != nullptr, "Scene is null (no owning scene)");
        FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
        return m_scene->m_registry.all_of<T>(m_entityHandle);
    }

    template <typename T, typename... Args>
    T &addComponent(Args &&...args) {
        FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
        FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
        FERMION_ASSERT(!hasComponent<T>(), "Entity already has this component");

        T &component = m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);

        if constexpr (std::is_same_v<T, CameraComponent>) {
            const uint32_t vw = m_scene->getViewportWidth();
            const uint32_t vh = m_scene->getViewportHeight();
            if (vw > 0 && vh > 0) {
                component.camera.setViewportSize(vw, vh);
            }
        }

        return component;
    }

    template <typename T>
    T &getComponent() {
        FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
        FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
        FERMION_ASSERT(hasComponent<T>(), "Entity does not have this component");
        return m_scene->m_registry.get<T>(m_entityHandle);
    }

    template <typename T>
    void removeComponent() {
        FERMION_ASSERT(m_scene != nullptr, "Entity is null (no owning scene)");
        FERMION_ASSERT(m_entityHandle != entt::null, "Entity handle is null");
        FERMION_ASSERT(hasComponent<T>(), "Entity does not have this component");
        m_scene->m_registry.remove<T>(m_entityHandle);
    }

    template <typename T, typename... Args>
    T &addOrReplaceComponent(Args &&...args) {
        T &component = m_scene->m_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
        if constexpr (std::is_same_v<T, CameraComponent>) {
            const uint32_t vw = m_scene->getViewportWidth();
            const uint32_t vh = m_scene->getViewportHeight();
            if (vw > 0 && vh > 0) {
                component.camera.setViewportSize(vw, vh);
            }
        }
        return component;
    }
    UUID getUUID() {
        return getComponent<IDComponent>().ID;
    }
    std::string getName() {
        return getComponent<TagComponent>().tag;
    }

private:
    entt::entity m_entityHandle{entt::null};
    Scene *m_scene{nullptr};
};

} // namespace Fermion
