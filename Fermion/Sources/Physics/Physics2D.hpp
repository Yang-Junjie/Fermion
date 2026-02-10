#pragma once

#include "Scene/Components.hpp"

#include <box2d/box2d.h>
#include <unordered_map>

namespace Fermion
{
    class Scene;
    class Entity;
    class EntityManager;

    namespace Utils
    {
        inline b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
        {
            switch (bodyType)
            {
            case Rigidbody2DComponent::BodyType::Static:
                return b2_staticBody;
            case Rigidbody2DComponent::BodyType::Dynamic:
                return b2_dynamicBody;
            case Rigidbody2DComponent::BodyType::Kinematic:
                return b2_kinematicBody;
            }

            FERMION_ASSERT(false, "Unknown body type");
            return b2_staticBody;
        }

        inline Rigidbody2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
        {
            switch (bodyType)
            {
            case b2_staticBody:
                return Rigidbody2DComponent::BodyType::Static;
            case b2_dynamicBody:
                return Rigidbody2DComponent::BodyType::Dynamic;
            case b2_kinematicBody:
                return Rigidbody2DComponent::BodyType::Kinematic;
            }
            FERMION_ASSERT(false, "Unknown body type");
            return Rigidbody2DComponent::BodyType::Static;
        }
    }

    class Physics2DWorld
    {
    public:
        Physics2DWorld() = default;
        ~Physics2DWorld();

        void start(Scene *scene);
        void stop();
        void step(Scene *scene, Timestep ts);

        void initSensor(Scene *scene, Entity entity);

        b2WorldId getWorld() const { return m_world; }
        bool isValid() const { return B2_IS_NON_NULL(m_world); }

        const std::unordered_map<UUID, b2BodyId> &getBodyMap() const { return m_bodyMap; }

    private:
        void createBodies(Scene *scene);
        void createJoints(Scene *scene);
        void syncKinematicBodies(Scene *scene);
        void stepWorld(Scene *scene, Timestep ts);
        void syncTransformsBack(Scene *scene);

        b2WorldId m_world = b2_nullWorldId;
        std::unordered_map<UUID, b2BodyId> m_bodyMap;
    };
} // namespace Fermion
