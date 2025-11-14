#include "fmpch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Renderer/Renderer2D.hpp"
#include <glm/glm.hpp>
#include "Scene.hpp"
#include "Core/Log.hpp"
namespace Fermion
{
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
    }

    void Scene::onUpdateEditor(Timestep ts, EditorCamera &camera)
    {
        Renderer2D::beginScene(camera);

        // non-owning group
        auto group = m_registry.group<>(entt::get<TransformComponent, SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto &transform = group.get<TransformComponent>(entity);
            auto &sprite = group.get<SpriteRendererComponent>(entity);
            // Renderer2D::drawQuad(transform.getTransform(), sprite.color);
            Renderer2D::drawSprite(transform.getTransform(), sprite,(int)entity);
        }

        Renderer2D::endScene();
    }

    void Scene::onUpdateRuntime(Timestep ts)
    {

        m_registry.view<NativeScriptComponent>().each(
            [=](auto entity, auto &nsc)
            {
                if (!nsc.instance)
                {
                    nsc.instance = nsc.instantiateScript();
                    nsc.instance->m_entity = Entity{entity, this};
                    nsc.instance->onCreate();
                }
                nsc.instance->onUpdate(ts);
            });

        Camera *mainCamera = nullptr;
        glm::mat4 cameraTransform;

        {
            auto view = m_registry.view<CameraComponent, TransformComponent>();
            for (auto entity : view)
            {
                auto &camera = view.get<CameraComponent>(entity);
                auto &transform = view.get<TransformComponent>(entity);
                if (camera.primary)
                {
                    mainCamera = &camera.camera;
                    cameraTransform = transform.getTransform();
                    break;
                }
            }
        }

        if (mainCamera)
        {
            Renderer2D::beginScene(*mainCamera, cameraTransform);

            // non-owning group
            auto group = m_registry.group<>(entt::get<TransformComponent, SpriteRendererComponent>);
            for (auto entity : group)
            {
                auto &transform = group.get<TransformComponent>(entity);
                auto &sprite = group.get<SpriteRendererComponent>(entity);
                Renderer2D::drawQuad(transform.getTransform(), sprite.color);
            }

            Renderer2D::endScene();
        }
    }

    void Scene::onViewportResize(uint32_t width, uint32_t height)
    {
        m_viewportWidth = width;
        m_viewportHeight = height;

        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto &cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.fixedAspectRatio)
            {
                cameraComponent.camera.setViewportSize(width, height);
            }
        }
    }
    Entity Scene::createEntity(std::string name)
    {
        Entity entity{m_registry.create(), this};
        entity.addComponent<TransformComponent>();
        entity.addComponent<TagComponent>(name.empty() ? std::string("unknown") : name);
        return entity;
    }
    void Scene::destroyEntity(Entity entity)
    {
        m_registry.destroy(entity);
    }

    Entity Scene::getPrimaryCameraEntity()
    {
        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto &camera = view.get<CameraComponent>(entity);
            if (camera.primary)
            {
                return Entity{entity, this};
            }
        }
        return {};
    }
}
