#include "fmpch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"
#include "Renderer/Renderer2D.hpp"
#include <glm/glm.hpp>
namespace Fermion
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::onUpdate(Timestep ts)
    {
        auto group = m_registry.group<TransformComponent, SpriteRendererComponent>();
        for (auto entity : group)
        {
            auto &transform = group.get<TransformComponent>(entity);
            auto &sprite = group.get<SpriteRendererComponent>(entity);
            Renderer2D::drawQuad(transform, sprite.color);
        }
    }

    entt::entity Scene::createEntity()
    {
        return m_registry.create();
    }
}
