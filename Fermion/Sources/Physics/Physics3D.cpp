#include "fmpch.hpp"
#include "Physics/Physics3D.hpp"

#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "Scene/Components.hpp"
#include "Core/Log.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/Core.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <algorithm>
#include <format>

namespace {
    namespace Physics3DLayers {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    } // namespace Physics3DLayers

    namespace BroadPhaseLayers3D {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::uint NUM_LAYERS = 2;
    } // namespace BroadPhaseLayers3D

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
            switch (inObject1) {
                case Physics3DLayers::NON_MOVING: return inObject2 == Physics3DLayers::MOVING;
                case Physics3DLayers::MOVING: return true;
                default: return false;
            }
        }
    };

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        BPLayerInterfaceImpl() {
            m_objectToBroadPhase[Physics3DLayers::NON_MOVING] = BroadPhaseLayers3D::NON_MOVING;
            m_objectToBroadPhase[Physics3DLayers::MOVING] = BroadPhaseLayers3D::MOVING;
        }

        JPH::uint GetNumBroadPhaseLayers() const override {
            return BroadPhaseLayers3D::NUM_LAYERS;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
            FERMION_ASSERT(inLayer < Physics3DLayers::NUM_LAYERS, "Invalid object layer");
            return m_objectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
            switch ((JPH::BroadPhaseLayer::Type) inLayer) {
                case (JPH::BroadPhaseLayer::Type) BroadPhaseLayers3D::NON_MOVING: return "NON_MOVING";
                case (JPH::BroadPhaseLayer::Type) BroadPhaseLayers3D::MOVING: return "MOVING";
                default: return "INVALID";
            }
        }
#endif

    private:
        JPH::BroadPhaseLayer m_objectToBroadPhase[Physics3DLayers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
            switch (inLayer1) {
                case Physics3DLayers::NON_MOVING: return inLayer2 == BroadPhaseLayers3D::MOVING;
                case Physics3DLayers::MOVING: return true;
                default: return false;
            }
        }
    };

    BPLayerInterfaceImpl s_broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl s_objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl s_objectLayerPairFilter;
    bool s_joltInitialized = false;

    void TraceImpl(const char *inFMT, ...) {
        char buffer[1024];
        va_list list;
        va_start(list, inFMT);
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);
        Fermion::Log::Debug(std::format("[Jolt] {}", buffer));
    }

#ifdef JPH_ENABLE_ASSERTS
    bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine) {
        Fermion::Log::Error(std::format("Jolt assert failed: {} ({}:{}): {}", inExpression ? inExpression : "",
                                        inFile ? inFile : "?", inLine, inMessage ? inMessage : ""));
        return true;
    }
#endif

    void ShutdownJolt() {
        if (!s_joltInitialized)
            return;

        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
        s_joltInitialized = false;
    }

    void InitializeJoltIfNeeded() {
        if (s_joltInitialized)
            return;

        JPH::RegisterDefaultAllocator();
        JPH::Trace = TraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        std::atexit(ShutdownJolt);

        s_joltInitialized = true;
    }

    JPH::EMotionType ToJoltMotionType(Fermion::Rigidbody3DComponent::BodyType type) {
        switch (type) {
            case Fermion::Rigidbody3DComponent::BodyType::Static: return JPH::EMotionType::Static;
            case Fermion::Rigidbody3DComponent::BodyType::Kinematic: return JPH::EMotionType::Kinematic;
            case Fermion::Rigidbody3DComponent::BodyType::Dynamic:
            default: return JPH::EMotionType::Dynamic;
        }
    }

    JPH::Vec3 ToJoltVec3(const glm::vec3 &value) {
        return {value.x, value.y, value.z};
    }

    JPH::Quat ToJoltQuat(const glm::quat &value) {
        return {value.x, value.y, value.z, value.w};
    }

    glm::vec3 ToGlmVec3(JPH::Vec3Arg value) {
        return {value.GetX(), value.GetY(), value.GetZ()};
    }

    glm::quat ToGlmQuat(JPH::QuatArg value) {
        return {value.GetW(), value.GetX(), value.GetY(), value.GetZ()};
    }

    JPH::ShapeRefC CreateBoxShape(const Fermion::TransformComponent &transform,
                                  const Fermion::BoxCollider3DComponent *collider) {
        glm::vec3 halfExtents = collider ? collider->size : glm::vec3{0.5f};
        halfExtents *= transform.scale;
        constexpr float cMinHalfExtent = 0.001f;
        halfExtents = glm::max(halfExtents, glm::vec3{cMinHalfExtent});

        JPH::BoxShapeSettings boxSettings(ToJoltVec3(halfExtents));
        JPH::ShapeSettings::ShapeResult result = boxSettings.Create();
        if (result.HasError()) {
            Fermion::Log::Error(std::format("Failed to create BoxShape: {}", result.GetError()));
            return nullptr;
        }
        return result.Get();
    }

    JPH::ShapeRefC CreateSphereShape(const Fermion::TransformComponent &transform,
                                     const Fermion::CircleCollider3DComponent *collider) {
        float radius = collider ? collider->radius : 0.5f;
        // Use the maximum scale component for uniform sphere scaling
        float maxScale = glm::max(glm::max(transform.scale.x, transform.scale.y), transform.scale.z);
        radius *= maxScale;
        constexpr float cMinRadius = 0.001f;
        radius = glm::max(radius, cMinRadius);

        JPH::SphereShapeSettings sphereSettings(radius);
        JPH::ShapeSettings::ShapeResult result = sphereSettings.Create();
        if (result.HasError()) {
            Fermion::Log::Error(std::format("Failed to create SphereShape: {}", result.GetError()));
            return nullptr;
        }
        return result.Get();
    }
} // namespace

namespace Fermion {
    Physics3DWorld::Physics3DWorld() = default;
    Physics3DWorld::~Physics3DWorld() = default;

    void Physics3DWorld::ensureInitialized() {
        if (!m_physicsSystem)
            InitializeJoltIfNeeded();
    }

    bool Physics3DWorld::isActive() const {
        return static_cast<bool>(m_physicsSystem);
    }

    void Physics3DWorld::start(Scene *scene) {
        FERMION_ASSERT(scene, "Scene is null");
        ensureInitialized();

        if (m_physicsSystem)
            stop(scene);

        const uint32_t maxBodies = 8192;
        const uint32_t maxBodyPairs = 65536;
        const uint32_t maxContactConstraints = 8192;

        if (!m_tempAllocator) {
            m_tempAllocator.reset(new JPH::TempAllocatorImpl(16 * 1024 * 1024));
        }

        if (!m_jobSystem) {
            uint32_t hardwareThreads = std::thread::hardware_concurrency();
            uint32_t workerCount = hardwareThreads > 1 ? hardwareThreads - 1 : 1;
            m_jobSystem.reset(new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, workerCount));
        }

        m_physicsSystem = std::make_unique<JPH::PhysicsSystem>();
        m_physicsSystem->Init(maxBodies, 0, maxBodyPairs, maxContactConstraints, s_broadPhaseLayerInterface,
                              s_objectVsBroadPhaseLayerFilter, s_objectLayerPairFilter);
        m_physicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

        auto view = scene->m_registry.view<Rigidbody3DComponent, TransformComponent>();
        for (auto entityID: view) {
            Entity entity{entityID, scene};
            auto &rb = view.get<Rigidbody3DComponent>(entityID);
            auto &transform = view.get<TransformComponent>(entityID);
            
            // Determine which collider type to use (prioritize BoxCollider, then CircleCollider)
            JPH::ShapeRefC shape;
            glm::vec3 offset{0.0f};
            float friction = 0.5f;
            float restitution = 0.0f;
            bool isTrigger = false;
            
            if (entity.hasComponent<BoxCollider3DComponent>()) {
                const auto &collider = entity.getComponent<BoxCollider3DComponent>();
                shape = CreateBoxShape(transform, &collider);
                offset = collider.offset;
                friction = collider.friction;
                restitution = collider.restitution;
                isTrigger = collider.isTrigger;
            }
            else if (entity.hasComponent<CircleCollider3DComponent>()) {
                const auto &collider = entity.getComponent<CircleCollider3DComponent>();
                shape = CreateSphereShape(transform, &collider);
                offset = collider.offset;
                friction = collider.friction;
                restitution = collider.restitution;
                isTrigger = collider.isTrigger;
            }
            else {
                // No collider component, skip
                continue;
            }

            if (!shape)
                continue;

            glm::quat rotationQuat = glm::quat(transform.getRotationEuler());
            glm::vec3 worldOffset = rotationQuat * offset;
            glm::vec3 bodyPosition = transform.translation + worldOffset;

            bool isStatic = rb.type == Rigidbody3DComponent::BodyType::Static;
            JPH::BodyCreationSettings bodySettings(shape, ToJoltVec3(bodyPosition), ToJoltQuat(rotationQuat),
                                                   ToJoltMotionType(rb.type),
                                                   isStatic ? Physics3DLayers::NON_MOVING : Physics3DLayers::MOVING);

            bodySettings.mAllowSleeping = true;
            bodySettings.mLinearDamping = rb.linearDamping;
            bodySettings.mAngularDamping = rb.angularDamping;
            bodySettings.mFriction = friction;
            bodySettings.mRestitution = restitution;
            bodySettings.mIsSensor = isTrigger;
            bodySettings.mGravityFactor = rb.useGravity ? 1.0f : 0.0f;
            bodySettings.mUserData = static_cast<uint64_t>(entity.getUUID());

            if (rb.mass > 0.0f) {
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
    }

    void Physics3DWorld::stop(Scene *scene) {
        if (!m_physicsSystem)
            return;

        JPH::BodyInterface &bodyInterface = m_physicsSystem->GetBodyInterface();
        for (auto &[uuid, storedId]: m_bodyMap) {
            if (storedId == 0)
                continue;
            JPH::BodyID bodyID(storedId);
            bodyInterface.RemoveBody(bodyID);
            bodyInterface.DestroyBody(bodyID);
        }
        m_bodyMap.clear();

        if (scene) {
            auto view = scene->m_registry.view<Rigidbody3DComponent>();
            for (auto entityID: view) {
                view.get<Rigidbody3DComponent>(entityID).runtimeBody = nullptr;
            }
        }

        m_physicsSystem.reset();
    }

    void Physics3DWorld::step(Scene *scene, Timestep ts) {
        if (!scene || !m_physicsSystem || !m_tempAllocator || !m_jobSystem)
            return;

        FM_PROFILE_FUNCTION();
        m_physicsSystem->Update(ts.getSeconds(), 1, m_tempAllocator.get(), m_jobSystem.get());

        const JPH::BodyLockInterfaceLocking &lockInterface = m_physicsSystem->GetBodyLockInterface();
        auto view = scene->m_registry.view<TransformComponent, Rigidbody3DComponent>();
        for (auto entityID: view) {
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
            glm::vec3 position = ToGlmVec3(body.GetCenterOfMassPosition());
            glm::quat rotation = ToGlmQuat(body.GetRotation());

            Entity entity{entityID, scene};
            glm::vec3 offset{0.0f};
            if (entity.hasComponent<BoxCollider3DComponent>()) {
                offset = entity.getComponent<BoxCollider3DComponent>().offset;
            }
            else if (entity.hasComponent<CircleCollider3DComponent>()) {
                offset = entity.getComponent<CircleCollider3DComponent>().offset;
            }
            glm::vec3 worldOffset = rotation * offset;

            auto &transform = view.get<TransformComponent>(entityID);
            transform.translation = position - worldOffset;
            transform.setRotationEuler(glm::eulerAngles(rotation));
        }
    }
} // namespace Fermion
