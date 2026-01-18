#pragma once
#include <entt/entt.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Scene/Entity.hpp"

namespace Fermion
{
    class EntityManager
    {
    public:
        explicit EntityManager(Scene *scene);

        entt::registry &getRegistry();
        const entt::registry &getRegistry() const;

        Entity createEntity(std::string name = std::string());
        Entity createChildEntity(Entity parent, std::string name = std::string());
        Entity createEntityWithUUID(UUID uuid, std::string name = std::string());

        void destroyEntity(Entity entity);
        Entity duplicateEntity(Entity entity);
        Entity findEntityByName(std::string_view name);
        Entity getEntityByUUID(UUID uuid);
        Entity getPrimaryCameraEntity();
        Entity tryGetEntityByUUID(UUID uuid);

        glm::mat4 getWorldSpaceTransformMatrix(Entity entity);
        TransformComponent getWorldSpaceTransform(Entity entity);
        void convertToWorldSpace(Entity entity);
        void convertToLocalSpace(Entity entity);

    private:
        template <typename... Component>
        static void copyComponentIfExists(Entity dst, Entity src)
        {
            ([&]()
             {
                if (src.hasComponent<Component>())
                    dst.addOrReplaceComponent<Component>(src.getComponent<Component>()); }(),
             ...);
        }

        template <typename... Component>
        static void copyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
        {
            copyComponentIfExists<Component...>(dst, src);
        }

    private:
        Scene *m_scene = nullptr;
        entt::registry m_registry;
        std::unordered_map<UUID, entt::entity> m_entityMap;
    };
} // namespace Fermion
