#include "Scene/Entity.hpp"
#include "Scene/EntityManager.hpp"

namespace Fermion
{
    Entity::Entity(entt::entity handle, Scene *scene) : m_entityHandle(handle), m_scene(scene)
    {
    }

    Entity Entity::getParent()
    {
        return m_scene->getEntityManager().getEntityByUUID(getParentUUID());
    }
} // namespace Fermion
