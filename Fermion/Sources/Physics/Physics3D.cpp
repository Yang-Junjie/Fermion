#include "fmpch.hpp"
#include "Physics/Physics3D.hpp"
#include "Physics/Physics3DJoltInit.hpp"
#include "Physics/Physics3DLayers.hpp"
#include "Physics/Physics3DTypes.hpp"
#include "Physics/Physics3DShapes.hpp"

#include "Scene/Scene.hpp"
#include "Scene/EntityManager.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Core/Log.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>

#include <thread>


namespace Fermion
{
    Physics3DWorld::Physics3DWorld() = default;
    Physics3DWorld::~Physics3DWorld() = default;

    void Physics3DWorld::ensureInitialized()
    {
        if (!m_physicsSystem)
            Physics3DInternal::InitializeJoltIfNeeded();
    }

    bool Physics3DWorld::isActive() const
    {
        return static_cast<bool>(m_physicsSystem);
    }

    void Physics3DWorld::start(Scene *scene)
    {
        FERMION_ASSERT(scene, "Scene is null");
        ensureInitialized();

        if (m_physicsSystem)
            stop(scene);

        const uint32_t maxBodies = 8192;
        const uint32_t maxBodyPairs = 65536;
        const uint32_t maxContactConstraints = 8192;

        if (!m_tempAllocator)
        {
            m_tempAllocator.reset(new JPH::TempAllocatorImpl(16 * 1024 * 1024));
        }

        if (!m_jobSystem)
        {
            uint32_t hardwareThreads = std::thread::hardware_concurrency();
            uint32_t workerCount = hardwareThreads > 1 ? hardwareThreads - 1 : 1;
            m_jobSystem.reset(new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, workerCount));
        }

        m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
        m_physicsSystem->Init(maxBodies, 0, maxBodyPairs, maxContactConstraints,
                              Physics3DInternal::GetBroadPhaseLayerInterface(),
                              Physics3DInternal::GetObjectVsBroadPhaseLayerFilter(),
                              Physics3DInternal::GetObjectLayerPairFilter());
        m_physicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

        auto view = scene->getRegistry().view<Rigidbody3DComponent, TransformComponent>();
        for (auto entityID : view)
        {
            Entity entity{entityID, scene};
            auto &rb = view.get<Rigidbody3DComponent>(entityID);
            TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);

            // Determine which collider type to use (prioritize BoxCollider, then CapsuleCollider, then CircleCollider)
            JPH::ShapeRefC shape;
            glm::vec3 offset{0.0f};
            float friction = 0.5f;
            float restitution = 0.0f;
            bool isTrigger = false;

            if (entity.hasComponent<BoxCollider3DComponent>())
            {
                const auto &collider = entity.getComponent<BoxCollider3DComponent>();
                shape = Physics3DShapes::CreateBoxShape(worldTransform, &collider);
                offset = collider.offset;
                friction = collider.friction;
                restitution = collider.restitution;
                isTrigger = collider.isTrigger;
            }
            else if (entity.hasComponent<CapsuleCollider3DComponent>())
            {
                const auto &collider = entity.getComponent<CapsuleCollider3DComponent>();
                shape = Physics3DShapes::CreateCapsuleShape(worldTransform, &collider);
                offset = collider.offset;
                friction = collider.friction;
                restitution = collider.restitution;
                isTrigger = collider.isTrigger;
            }
            else if (entity.hasComponent<CircleCollider3DComponent>())
            {
                const auto &collider = entity.getComponent<CircleCollider3DComponent>();
                shape = Physics3DShapes::CreateSphereShape(worldTransform, &collider);
                offset = collider.offset;
                friction = collider.friction;
                restitution = collider.restitution;
                isTrigger = collider.isTrigger;
            }
            else if (entity.hasComponent<MeshCollider3DComponent>())
            {
                const auto &collider = entity.getComponent<MeshCollider3DComponent>();
                shape = Physics3DShapes::CreateMeshShape(worldTransform, &collider);
                offset = collider.offset;
                friction = collider.friction;
                restitution = collider.restitution;
                isTrigger = collider.isTrigger;
            }
            else
            {
                // No collider component, skip
                continue;
            }

            if (!shape)
                continue;

            glm::quat rotationQuat = glm::quat(worldTransform.getRotationEuler());
            glm::vec3 worldOffset = rotationQuat * offset;
            glm::vec3 bodyPosition = worldTransform.translation + worldOffset;

            bool isStatic = rb.type == Rigidbody3DComponent::BodyType::Static;
            JPH::BodyCreationSettings bodySettings(shape, Physics3DUtils::ToJoltVec3(bodyPosition),
                                                   Physics3DUtils::ToJoltQuat(rotationQuat),
                                                   Physics3DUtils::ToJoltMotionType(rb.type),
                                                   isStatic ? Physics3DInternal::Layers::NON_MOVING
                                                            : Physics3DInternal::Layers::MOVING);

            bodySettings.mAllowSleeping = true;
            bodySettings.mLinearDamping = rb.linearDamping;
            bodySettings.mAngularDamping = rb.angularDamping;
            bodySettings.mFriction = friction;
            bodySettings.mRestitution = restitution;
            bodySettings.mIsSensor = isTrigger;
            bodySettings.mGravityFactor = rb.useGravity ? 1.0f : 0.0f;
            bodySettings.mUserData = static_cast<uint64_t>(entity.getUUID());
            bodySettings.mAllowedDOFs = rb.fixedRotation
                                            ? (JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY |
                                               JPH::EAllowedDOFs::TranslationZ)
                                            : JPH::EAllowedDOFs::All;

            if (rb.mass > 0.0f)
            {
                bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
                JPH::MassProperties massProperties = shape->GetMassProperties();
                massProperties.ScaleToMass(rb.mass);
                bodySettings.mMassPropertiesOverride = massProperties;
            }

            auto activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;
            JPH::BodyInterface &bodyInterface = m_physicsSystem->GetBodyInterface();
            JPH::BodyID bodyID = bodyInterface.CreateAndAddBody(bodySettings, activation);

            if (bodyID.IsInvalid())
                continue;

            m_bodyMap[entity.getUUID()] = bodyID.GetIndexAndSequenceNumber();
            rb.runtimeBody = reinterpret_cast<void *>(static_cast<uint64_t>(bodyID.GetIndexAndSequenceNumber()));
        }

        // Create hinge constraints after all bodies are created
        {
            auto constraintView = scene->getRegistry().view<HingeConstraint3DComponent>();
            JPH::BodyInterface &bodyInterface = m_physicsSystem->GetBodyInterface();
            for (auto entityID : constraintView)
            {
                Entity entity{entityID, scene};
                auto &hc = constraintView.get<HingeConstraint3DComponent>(entityID);

                if (static_cast<uint64_t>(hc.connectedBodyID) == 0)
                    continue;

                auto itA = m_bodyMap.find(entity.getUUID());
                auto itB = m_bodyMap.find(hc.connectedBodyID);
                if (itA == m_bodyMap.end() || itB == m_bodyMap.end())
                    continue;

                JPH::BodyID bodyIDA(itA->second);
                JPH::BodyID bodyIDB(itB->second);

                JPH::HingeConstraintSettings settings;
                settings.mPoint1 = Physics3DUtils::ToJoltVec3(hc.localAnchorA);
                settings.mPoint2 = Physics3DUtils::ToJoltVec3(hc.localAnchorB);
                settings.mHingeAxis1 = Physics3DUtils::ToJoltVec3(glm::normalize(hc.hingeAxisA));
                settings.mHingeAxis2 = Physics3DUtils::ToJoltVec3(glm::normalize(hc.hingeAxisB));
                settings.mSpace = JPH::EConstraintSpace::LocalToBodyCOM;

                if (hc.enableLimit)
                {
                    settings.mLimitsMin = hc.lowerAngle;
                    settings.mLimitsMax = hc.upperAngle;
                }

                JPH::Body *bodyA = nullptr;
                JPH::Body *bodyB = nullptr;
                {
                    JPH::BodyLockWrite lockA(m_physicsSystem->GetBodyLockInterface(), bodyIDA);
                    if (lockA.Succeeded())
                        bodyA = &lockA.GetBody();
                }
                {
                    JPH::BodyLockWrite lockB(m_physicsSystem->GetBodyLockInterface(), bodyIDB);
                    if (lockB.Succeeded())
                        bodyB = &lockB.GetBody();
                }

                if (!bodyA || !bodyB)
                    continue;

                JPH::Constraint *constraint = settings.Create(*bodyA, *bodyB);
                m_physicsSystem->AddConstraint(constraint);

                if (hc.enableMotor)
                {
                    auto *hingeConstraint = static_cast<JPH::HingeConstraint *>(constraint);
                    hingeConstraint->SetMotorState(JPH::EMotorState::Velocity);
                    JPH::MotorSettings &motorSettings = hingeConstraint->GetMotorSettings();
                    motorSettings.mMaxTorqueLimit = hc.maxMotorTorque;
                    motorSettings.mMinTorqueLimit = -hc.maxMotorTorque;
                    hingeConstraint->SetTargetAngularVelocity(hc.motorSpeed);
                }

                hc.runtimeConstraint = constraint;
                constraint->AddRef();
                m_constraintList.push_back(constraint);
            }
        }
    }

    void Physics3DWorld::stop(Scene *scene)
    {
        if (!m_physicsSystem)
            return;

        // Remove constraints first
        for (auto *constraint : m_constraintList)
        {
            m_physicsSystem->RemoveConstraint(constraint);
            constraint->Release();
        }
        m_constraintList.clear();

        if (scene)
        {
            auto constraintView = scene->getRegistry().view<HingeConstraint3DComponent>();
            for (auto entityID : constraintView)
            {
                constraintView.get<HingeConstraint3DComponent>(entityID).runtimeConstraint = nullptr;
            }
        }

        JPH::BodyInterface &bodyInterface = m_physicsSystem->GetBodyInterface();
        for (auto &[uuid, storedId] : m_bodyMap)
        {
            if (storedId == 0)
                continue;
            JPH::BodyID bodyID(storedId);
            bodyInterface.RemoveBody(bodyID);
            bodyInterface.DestroyBody(bodyID);
        }
        m_bodyMap.clear();

        if (scene)
        {
            auto view = scene->getRegistry().view<Rigidbody3DComponent>();
            for (auto entityID : view)
            {
                view.get<Rigidbody3DComponent>(entityID).runtimeBody = nullptr;
            }
        }

        m_physicsSystem.reset();
    }

    void Physics3DWorld::addBody(Scene *scene, Entity entity)
    {
        if (!scene || !m_physicsSystem || !entity)
            return;

        if (!entity.hasComponent<Rigidbody3DComponent>() || !entity.hasComponent<TransformComponent>())
            return;

        auto &rb = entity.getComponent<Rigidbody3DComponent>();

        // Skip if already has a runtime body
        if (rb.runtimeBody)
            return;

        TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);

        JPH::ShapeRefC shape;
        glm::vec3 offset{0.0f};
        float friction = 0.5f;
        float restitution = 0.0f;
        bool isTrigger = false;

        if (entity.hasComponent<BoxCollider3DComponent>())
        {
            const auto &collider = entity.getComponent<BoxCollider3DComponent>();
            shape = Physics3DShapes::CreateBoxShape(worldTransform, &collider);
            offset = collider.offset;
            friction = collider.friction;
            restitution = collider.restitution;
            isTrigger = collider.isTrigger;
        }
        else if (entity.hasComponent<CapsuleCollider3DComponent>())
        {
            const auto &collider = entity.getComponent<CapsuleCollider3DComponent>();
            shape = Physics3DShapes::CreateCapsuleShape(worldTransform, &collider);
            offset = collider.offset;
            friction = collider.friction;
            restitution = collider.restitution;
            isTrigger = collider.isTrigger;
        }
        else if (entity.hasComponent<CircleCollider3DComponent>())
        {
            const auto &collider = entity.getComponent<CircleCollider3DComponent>();
            shape = Physics3DShapes::CreateSphereShape(worldTransform, &collider);
            offset = collider.offset;
            friction = collider.friction;
            restitution = collider.restitution;
            isTrigger = collider.isTrigger;
        }
        else if (entity.hasComponent<MeshCollider3DComponent>())
        {
            const auto &collider = entity.getComponent<MeshCollider3DComponent>();
            shape = Physics3DShapes::CreateMeshShape(worldTransform, &collider);
            offset = collider.offset;
            friction = collider.friction;
            restitution = collider.restitution;
            isTrigger = collider.isTrigger;
        }
        else
        {
            return;
        }

        if (!shape)
            return;

        glm::quat rotationQuat = glm::quat(worldTransform.getRotationEuler());
        glm::vec3 worldOffset = rotationQuat * offset;
        glm::vec3 bodyPosition = worldTransform.translation + worldOffset;

        bool isStatic = rb.type == Rigidbody3DComponent::BodyType::Static;
        JPH::BodyCreationSettings bodySettings(shape, Physics3DUtils::ToJoltVec3(bodyPosition),
                                               Physics3DUtils::ToJoltQuat(rotationQuat),
                                               Physics3DUtils::ToJoltMotionType(rb.type),
                                               isStatic ? Physics3DInternal::Layers::NON_MOVING
                                                        : Physics3DInternal::Layers::MOVING);

        bodySettings.mAllowSleeping = true;
        bodySettings.mLinearDamping = rb.linearDamping;
        bodySettings.mAngularDamping = rb.angularDamping;
        bodySettings.mFriction = friction;
        bodySettings.mRestitution = restitution;
        bodySettings.mIsSensor = isTrigger;
        bodySettings.mGravityFactor = rb.useGravity ? 1.0f : 0.0f;
        bodySettings.mUserData = static_cast<uint64_t>(entity.getUUID());
        bodySettings.mAllowedDOFs = rb.fixedRotation
                                        ? (JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY |
                                           JPH::EAllowedDOFs::TranslationZ)
                                        : JPH::EAllowedDOFs::All;

        if (rb.mass > 0.0f)
        {
            bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            JPH::MassProperties massProperties = shape->GetMassProperties();
            massProperties.ScaleToMass(rb.mass);
            bodySettings.mMassPropertiesOverride = massProperties;
        }

        auto activation = isStatic ? JPH::EActivation::DontActivate : JPH::EActivation::Activate;
        JPH::BodyInterface &bodyInterface = m_physicsSystem->GetBodyInterface();
        JPH::BodyID bodyID = bodyInterface.CreateAndAddBody(bodySettings, activation);

        if (bodyID.IsInvalid())
            return;

        m_bodyMap[entity.getUUID()] = bodyID.GetIndexAndSequenceNumber();
        rb.runtimeBody = reinterpret_cast<void *>(static_cast<uint64_t>(bodyID.GetIndexAndSequenceNumber()));
    }

    void Physics3DWorld::step(Scene *scene, Timestep ts)
    {
        if (!scene || !m_physicsSystem || !m_tempAllocator || !m_jobSystem)
            return;

        FM_PROFILE_FUNCTION();
        const float deltaTime = ts.getSeconds();
        {
            // Sync kinematic bodies from Transform before stepping physics.
            JPH::BodyInterface &bodyInterface = m_physicsSystem->GetBodyInterface();
            auto view = scene->getRegistry().view<TransformComponent, Rigidbody3DComponent>();
            for (auto entityID : view)
            {
                auto &rb = view.get<Rigidbody3DComponent>(entityID);
                if (rb.type != Rigidbody3DComponent::BodyType::Kinematic)
                    continue;
                if (!rb.runtimeBody)
                    continue;

                uint64_t storedValue = reinterpret_cast<uint64_t>(rb.runtimeBody);
                if (storedValue == 0)
                    continue;
                JPH::BodyID bodyID(static_cast<uint32_t>(storedValue));
                if (!bodyInterface.IsAdded(bodyID))
                    continue;

                Entity entity{entityID, scene};
                TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);
                glm::quat rotationQuat = glm::quat(worldTransform.getRotationEuler());
                glm::vec3 offset{0.0f};
                if (entity.hasComponent<BoxCollider3DComponent>())
                {
                    offset = entity.getComponent<BoxCollider3DComponent>().offset;
                }
                else if (entity.hasComponent<CapsuleCollider3DComponent>())
                {
                    offset = entity.getComponent<CapsuleCollider3DComponent>().offset;
                }
                else if (entity.hasComponent<CircleCollider3DComponent>())
                {
                    offset = entity.getComponent<CircleCollider3DComponent>().offset;
                }
                else if (entity.hasComponent<MeshCollider3DComponent>())
                {
                    offset = entity.getComponent<MeshCollider3DComponent>().offset;
                }
                glm::vec3 worldOffset = rotationQuat * offset;
                glm::vec3 bodyPosition = worldTransform.translation + worldOffset;

                if (deltaTime > 0.0f)
                {
                    bodyInterface.MoveKinematic(bodyID, Physics3DUtils::ToJoltVec3(bodyPosition),
                                                Physics3DUtils::ToJoltQuat(rotationQuat), deltaTime);
                }
                else
                {
                    bodyInterface.SetPositionAndRotation(bodyID, Physics3DUtils::ToJoltVec3(bodyPosition),
                                                         Physics3DUtils::ToJoltQuat(rotationQuat),
                                                         JPH::EActivation::DontActivate);
                }
            }
        }
        m_physicsSystem->Update(ts.getSeconds(), 1, m_tempAllocator.get(), m_jobSystem.get());

        const JPH::BodyLockInterfaceLocking &lockInterface = m_physicsSystem->GetBodyLockInterface();
        auto view = scene->getRegistry().view<TransformComponent, Rigidbody3DComponent>();
        for (auto entityID : view)
        {
            auto &rb = view.get<Rigidbody3DComponent>(entityID);
            if (!rb.runtimeBody)
                continue;

            uint64_t storedValue = reinterpret_cast<uint64_t>(rb.runtimeBody);
            if (storedValue == 0)
                continue;
            JPH::BodyID bodyID(static_cast<uint32_t>(storedValue));

            JPH::BodyLockRead lock(lockInterface, bodyID);
            if (!lock.Succeeded())
                continue;

            const JPH::Body &body = lock.GetBody();
            glm::vec3 position = Physics3DUtils::ToGlmVec3(body.GetCenterOfMassPosition());
            glm::quat rotation = Physics3DUtils::ToGlmQuat(body.GetRotation());

            Entity entity{entityID, scene};
            glm::vec3 offset{0.0f};
            if (entity.hasComponent<BoxCollider3DComponent>())
            {
                offset = entity.getComponent<BoxCollider3DComponent>().offset;
            }
            else if (entity.hasComponent<CapsuleCollider3DComponent>())
            {
                offset = entity.getComponent<CapsuleCollider3DComponent>().offset;
            }
            else if (entity.hasComponent<CircleCollider3DComponent>())
            {
                offset = entity.getComponent<CircleCollider3DComponent>().offset;
            }
            else if (entity.hasComponent<MeshCollider3DComponent>())
            {
                offset = entity.getComponent<MeshCollider3DComponent>().offset;
            }
            glm::vec3 worldOffset = rotation * offset;

            TransformComponent worldTransform = scene->getEntityManager().getWorldSpaceTransform(entity);
            worldTransform.translation = position - worldOffset;
            worldTransform.setRotationEuler(glm::eulerAngles(rotation));

            auto &transform = view.get<TransformComponent>(entityID);
            transform.setTransform(worldTransform.getTransform());
            scene->getEntityManager().convertToLocalSpace(entity);
        }
    }
} // namespace Fermion
