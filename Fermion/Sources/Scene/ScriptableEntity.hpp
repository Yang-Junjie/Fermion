#pragma once
#include "Scene/Entity.hpp"
namespace Fermion {
class ScriptableEntity {
public:
    virtual ~ScriptableEntity() = default;
    template <typename T>
    T &getComponent() {
        return m_entity.getComponent<T>();
    }

    virtual void onCreate() {
    }
    virtual void onDestroy() {
    }
    virtual void onUpdate(Timestep ts) {
    }

private:
    Entity m_entity;
    friend class Scene;
};
} // namespace Fermion