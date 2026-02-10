#include "fmpch.hpp"
#include "Physics/Physics2D.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "Scene/EntityManager.hpp"
#include "Core/Log.hpp"

namespace Fermion
{
    Physics2DWorld::~Physics2DWorld()
    {
        stop();
    }

    void Physics2DWorld::start(Scene *scene)
    {
        FM_PROFILE_FUNCTION();
        b2WorldDef worldDef = b2DefaultWorldDef();
        worldDef.gravity = {0.0f, -9.8f};
        m_world = b2CreateWorld(&worldDef);

        createBodies(scene);
        createJoints(scene);
    }

    void Physics2DWorld::stop()
    {
        if (B2_IS_NON_NULL(m_world))
        {
            b2DestroyWorld(m_world);
            m_world = b2_nullWorldId;
        }
        m_bodyMap.clear();
    }

    void Physics2DWorld::step(Scene *scene, Timestep ts)
    {
        if (!isValid())
            return;

        syncKinematicBodies(scene);
        stepWorld(scene, ts);
        syncTransformsBack(scene);
    }

    void Physics2DWorld::initSensor(Scene *scene, Entity entity)
    {
        TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);
        auto &boxSensor2d = entity.getComponent<BoxSensor2DComponent>();

        float halfWidth = boxSensor2d.size.x * worldTransform.scale.x;
        float halfHeight = boxSensor2d.size.y * worldTransform.scale.y;

        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.isSensor = true;
        shapeDef.enableSensorEvents = true;

        b2Polygon box = b2MakeOffsetBox(
            halfWidth, halfHeight,
            b2Vec2{boxSensor2d.offset.x, boxSensor2d.offset.y},
            b2Rot_identity);

        b2ShapeId shapeId = b2CreatePolygonShape(m_bodyMap[entity.getUUID()], &shapeDef, &box);
        boxSensor2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
    }

    void Physics2DWorld::createBodies(Scene *scene)
    {
        auto &registry = scene->getRegistry();
        auto view = registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity{e, scene};
            TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);
            auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = Utils::Rigidbody2DTypeToBox2DBody(rb2d.type);
            bodyDef.position = {worldTransform.translation.x, worldTransform.translation.y};
            bodyDef.rotation = b2MakeRot(worldTransform.rotation.z);

            b2BodyId bodyId = b2CreateBody(m_world, &bodyDef);
            m_bodyMap[entity.getUUID()] = bodyId;

            if (rb2d.fixedRotation)
            {
                b2MotionLocks locks = {false, false, true};
                b2Body_SetMotionLocks(bodyId, locks);
            }

            rb2d.runtimeBody = (void *)(uintptr_t)b2StoreBodyId(bodyId);

            if (entity.hasComponent<BoxCollider2DComponent>())
            {
                auto &bc2d = entity.getComponent<BoxCollider2DComponent>();

                float halfWidth = bc2d.size.x * worldTransform.scale.x;
                float halfHeight = bc2d.size.y * worldTransform.scale.y;

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                shapeDef.density = bc2d.density;
                shapeDef.material.friction = bc2d.friction;
                shapeDef.material.restitution = bc2d.restitution;
                shapeDef.enableSensorEvents = true;

                b2Polygon box = b2MakeOffsetBox(
                    halfWidth, halfHeight,
                    b2Vec2{bc2d.offset.x, bc2d.offset.y},
                    b2Rot_identity);

                b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                bc2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
            }

            if (entity.hasComponent<CircleCollider2DComponent>())
            {
                auto &cc2d = entity.getComponent<CircleCollider2DComponent>();

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                shapeDef.density = cc2d.density;
                shapeDef.material.friction = cc2d.friction;
                shapeDef.material.restitution = cc2d.restitution;
                shapeDef.enableSensorEvents = true;

                b2Circle circle;
                circle.center = b2Vec2{
                    cc2d.offset.x * worldTransform.scale.x,
                    cc2d.offset.y * worldTransform.scale.y};
                circle.radius = cc2d.radius * worldTransform.scale.x;

                b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
                cc2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
            }

            if (entity.hasComponent<CapsuleCollider2DComponent>())
            {
                auto &cc2d = entity.getComponent<CapsuleCollider2DComponent>();

                b2ShapeDef shapeDef = b2DefaultShapeDef();
                shapeDef.density = cc2d.density;
                shapeDef.material.friction = cc2d.friction;
                shapeDef.material.restitution = cc2d.restitution;
                shapeDef.enableSensorEvents = true;

                b2Capsule capsule;
                float scaleX = worldTransform.scale.x;
                float scaleY = worldTransform.scale.y;
                float scaledRadius = cc2d.radius * std::abs(scaleX);
                float scaledHeight = cc2d.height * std::abs(scaleY);
                float halfSegmentHeight = std::max(0.0f, (scaledHeight / 2.0f) - scaledRadius);
                float offsetX = cc2d.offset.x * scaleX;
                float offsetY = cc2d.offset.y * scaleY;

                capsule.center1 = b2Vec2{offsetX, offsetY - halfSegmentHeight};
                capsule.center2 = b2Vec2{offsetX, offsetY + halfSegmentHeight};
                capsule.radius = scaledRadius;

                b2ShapeId shapeId = b2CreateCapsuleShape(bodyId, &shapeDef, &capsule);
                cc2d.runtimeFixture = (void *)(uintptr_t)b2StoreShapeId(shapeId);
            }

            if (entity.hasComponent<BoxSensor2DComponent>())
            {
                initSensor(scene, entity);
            }
        }
    }

    void Physics2DWorld::createJoints(Scene *scene)
    {
        auto &registry = scene->getRegistry();

        // Revolute joints
        {
            auto jointView = registry.view<RevoluteJoint2DComponent>();
            for (auto e : jointView)
            {
                Entity entity{e, scene};
                if (!entity.hasComponent<Rigidbody2DComponent>())
                    continue;

                auto &joint = entity.getComponent<RevoluteJoint2DComponent>();
                if (static_cast<uint64_t>(joint.connectedBodyID) == 0)
                    continue;

                auto itA = m_bodyMap.find(entity.getUUID());
                auto itB = m_bodyMap.find(joint.connectedBodyID);
                if (itA == m_bodyMap.end() || itB == m_bodyMap.end())
                    continue;

                b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
                jointDef.base.bodyIdA = itA->second;
                jointDef.base.bodyIdB = itB->second;
                jointDef.base.localFrameA.p = b2Vec2{joint.localAnchorA.x, joint.localAnchorA.y};
                jointDef.base.localFrameA.q = b2Rot_identity;
                jointDef.base.localFrameB.p = b2Vec2{joint.localAnchorB.x, joint.localAnchorB.y};
                jointDef.base.localFrameB.q = b2Rot_identity;
                jointDef.enableLimit = joint.enableLimit;
                jointDef.lowerAngle = joint.lowerAngle;
                jointDef.upperAngle = joint.upperAngle;
                jointDef.enableMotor = joint.enableMotor;
                jointDef.motorSpeed = joint.motorSpeed;
                jointDef.maxMotorTorque = joint.maxMotorTorque;

                b2JointId jointId = b2CreateRevoluteJoint(m_world, &jointDef);
                joint.runtimeJoint = (void *)(uintptr_t)b2StoreJointId(jointId);
            }
        }

        // Distance joints
        {
            auto jointView = registry.view<DistanceJoint2DComponent>();
            for (auto e : jointView)
            {
                Entity entity{e, scene};
                if (!entity.hasComponent<Rigidbody2DComponent>())
                    continue;

                auto &joint = entity.getComponent<DistanceJoint2DComponent>();
                if (static_cast<uint64_t>(joint.connectedBodyID) == 0)
                    continue;

                auto itA = m_bodyMap.find(entity.getUUID());
                auto itB = m_bodyMap.find(joint.connectedBodyID);
                if (itA == m_bodyMap.end() || itB == m_bodyMap.end())
                    continue;

                b2DistanceJointDef jointDef = b2DefaultDistanceJointDef();
                jointDef.base.bodyIdA = itA->second;
                jointDef.base.bodyIdB = itB->second;
                jointDef.base.localFrameA.p = b2Vec2{joint.localAnchorA.x, joint.localAnchorA.y};
                jointDef.base.localFrameA.q = b2Rot_identity;
                jointDef.base.localFrameB.p = b2Vec2{joint.localAnchorB.x, joint.localAnchorB.y};
                jointDef.base.localFrameB.q = b2Rot_identity;
                jointDef.length = joint.length;
                jointDef.enableSpring = joint.enableSpring;
                jointDef.hertz = joint.hertz;
                jointDef.dampingRatio = joint.damping;
                jointDef.enableLimit = joint.enableLimit;
                jointDef.minLength = joint.minLength;
                jointDef.maxLength = joint.maxLength;

                b2JointId jointId = b2CreateDistanceJoint(m_world, &jointDef);
                joint.runtimeJoint = (void *)(uintptr_t)b2StoreJointId(jointId);
            }
        }
    }

    void Physics2DWorld::syncKinematicBodies(Scene *scene)
    {
        auto &registry = scene->getRegistry();
        auto view = registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity{e, scene};
            auto &rb2d = entity.getComponent<Rigidbody2DComponent>();
            if (rb2d.type != Rigidbody2DComponent::BodyType::Kinematic)
                continue;

            if (!rb2d.runtimeBody)
                continue;

            uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
            b2BodyId bodyId = b2LoadBodyId(storedId);
            if (!b2Body_IsValid(bodyId))
                continue;

            TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);
            b2Body_SetTransform(bodyId,
                                {worldTransform.translation.x, worldTransform.translation.y},
                                b2MakeRot(worldTransform.rotation.z));
        }
    }

    void Physics2DWorld::stepWorld(Scene *scene, Timestep ts)
    {
        auto &registry = scene->getRegistry();

        b2World_Step(m_world, ts.getSeconds(), 4);
        b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_world);

        auto view = registry.view<BoxSensor2DComponent>();
        for (auto e : view)
        {
            Entity entity{e, scene};
            auto &bs2d = entity.getComponent<BoxSensor2DComponent>();

            if (!bs2d.runtimeFixture)
                continue;

            uint64_t storedShapeId = (uint64_t)(uintptr_t)bs2d.runtimeFixture;
            b2ShapeId myShapeId = b2LoadShapeId(storedShapeId);

            if (!b2Shape_IsValid(myShapeId))
                continue;

            bs2d.sensorBegin = false;
            for (int i = 0; i < sensorEvents.beginCount; ++i)
            {
                b2SensorBeginTouchEvent *begin = sensorEvents.beginEvents + i;
                if (B2_ID_EQUALS(begin->sensorShapeId, myShapeId))
                {
                    bs2d.sensorBegin = true;
                    break;
                }
            }
            bs2d.sensorEnd = false;
            for (int i = 0; i < sensorEvents.endCount; ++i)
            {
                b2SensorEndTouchEvent *end = sensorEvents.endEvents + i;
                if (b2Shape_IsValid(end->sensorShapeId) &&
                    B2_ID_EQUALS(end->sensorShapeId, myShapeId))
                {
                    bs2d.sensorEnd = true;
                    break;
                }
            }
        }
    }

    void Physics2DWorld::syncTransformsBack(Scene *scene)
    {
        auto &registry = scene->getRegistry();
        auto view = registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity{e, scene};
            auto &rb2d = entity.getComponent<Rigidbody2DComponent>();

            if (!rb2d.runtimeBody)
                continue;

            uint64_t storedId = (uint64_t)(uintptr_t)rb2d.runtimeBody;
            b2BodyId bodyId = b2LoadBodyId(storedId);
            if (!b2Body_IsValid(bodyId))
                continue;

            b2Transform xf = b2Body_GetTransform(bodyId);

            float angle = atan2f(xf.q.s, xf.q.c);
            TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);
            worldTransform.translation.x = xf.p.x;
            worldTransform.translation.y = xf.p.y;
            worldTransform.rotation.z = angle;
            auto &transform = entity.getComponent<TransformComponent>();
            transform.setTransform(worldTransform.getTransform());
            scene->getEntityManager().convertToLocalSpace(entity);
        }
    }
} // namespace Fermion
