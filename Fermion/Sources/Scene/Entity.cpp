#include "Scene/Entity.hpp"
#include "Components.hpp"
namespace Fermion
{
    Entity::Entity(entt::entity handle, Scene *scene)
        : m_entityHandle(handle), m_scene(scene) {}



}