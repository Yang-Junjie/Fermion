#include "fmpch.hpp"
#include "Scene/EntityManager.hpp"

namespace Fermion
{
    EntityManager::EntityManager(Scene *scene) : m_scene(scene)
    {
    }

    entt::registry &EntityManager::getRegistry()
    {
        return m_registry;
    }

    const entt::registry &EntityManager::getRegistry() const
    {
        return m_registry;
    }

    Entity EntityManager::createEntity(std::string name)
    {
        return createEntityWithUUID(UUID(), std::move(name));
    }

    Entity EntityManager::createChildEntity(Entity parent, std::string name)
    {
        Entity entity = createEntity(std::move(name));
        if (parent)
            entity.setParent(parent);
        return entity;
    }

    Entity EntityManager::createEntityWithUUID(UUID uuid, std::string name)
    {
        Entity entity{m_registry.create(), m_scene};
        auto &idComponent = entity.addComponent<IDComponent>();
        idComponent.ID = uuid;

        entity.addComponent<TransformComponent>();
        if (!name.empty())
            entity.addComponent<TagComponent>(name);

        entity.addComponent<RelationshipComponent>();
        m_entityMap[uuid] = entity;

        return entity;
    }

    void EntityManager::destroyEntity(Entity entity)
    {
        if (!entity)
            return;

        Entity parent = tryGetEntityByUUID(entity.getParentUUID());
        if (parent)
            parent.removeChild(entity);

        std::vector<UUID> children = entity.getChildren();
        for (UUID childId : children)
        {
            Entity child = tryGetEntityByUUID(childId);
            if (child)
                destroyEntity(child);
        }

        m_entityMap.erase(entity.getUUID());
        m_registry.destroy(entity);
    }

    Entity EntityManager::duplicateEntity(Entity entity)
    {
        std::string name = entity.getName();
        Entity newEntity = createEntity(name);
        copyComponentIfExists(AllComponents{}, newEntity, entity);
        newEntity.getComponent<RelationshipComponent>() = {};
        Entity parent = tryGetEntityByUUID(entity.getParentUUID());
        if (parent)
            newEntity.setParent(parent);
        return newEntity;
    }

    Entity EntityManager::findEntityByName(std::string_view name)
    {
        auto view = m_registry.view<TagComponent>();
        for (auto entity : view)
        {
            const TagComponent &tc = view.get<TagComponent>(entity);
            if (tc.tag == name)
                return Entity{entity, m_scene};
        }
        return {};
    }

    Entity EntityManager::getEntityByUUID(UUID uuid)
    {
        if (m_entityMap.find(uuid) != m_entityMap.end())
            return {m_entityMap.at(uuid), m_scene};

        return {};
    }

    Entity EntityManager::getPrimaryCameraEntity()
    {
        auto view = m_registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto &camera = view.get<CameraComponent>(entity);
            if (camera.primary)
            {
                return Entity{entity, m_scene};
            }
        }
        return {};
    }

    Entity EntityManager::tryGetEntityByUUID(UUID uuid)
    {
        if (const auto iter = m_entityMap.find(uuid); iter != m_entityMap.end())
            return {iter->second, m_scene};
        return Entity{};
    }

    glm::mat4 EntityManager::getWorldSpaceTransformMatrix(Entity entity)
    {
        glm::mat4 transform(1.0f);
        Entity parent = tryGetEntityByUUID(entity.getParentUUID());
        if (parent)
            transform = getWorldSpaceTransformMatrix(parent);

        return transform * entity.Transform().getTransform();
    }

    TransformComponent EntityManager::getWorldSpaceTransform(Entity entity)
    {
        glm::mat4 transform = getWorldSpaceTransformMatrix(entity);
        TransformComponent transformComponent;
        transformComponent.setTransform(transform);
        return transformComponent;
    }

    void EntityManager::convertToWorldSpace(Entity entity)
    {
        Entity parent = tryGetEntityByUUID(entity.getParentUUID());

        if (!parent)
            return;

        glm::mat4 transform = getWorldSpaceTransformMatrix(entity);
        auto &entityTransform = entity.Transform();
        entityTransform.setTransform(transform);
    }

    void EntityManager::convertToLocalSpace(Entity entity)
    {
        Entity parent = tryGetEntityByUUID(entity.getParentUUID());

        if (!parent)
            return;

        auto &transform = entity.Transform();
        glm::mat4 parentTransform = getWorldSpaceTransformMatrix(parent);
        glm::mat4 localTransform = glm::inverse(parentTransform) * transform.getTransform();
        transform.setTransform(localTransform);
    }
} // namespace Fermion
