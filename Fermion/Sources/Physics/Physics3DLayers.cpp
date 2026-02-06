#include "fmpch.hpp"
#include "Physics/Physics3DLayers.hpp"

namespace
{
    using namespace Fermion::Physics3DInternal;

    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                return false;
            }
        }
    };

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            m_objectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            m_objectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        JPH::uint GetNumBroadPhaseLayers() const override
        {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
        {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return m_objectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        const char *GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
        {
            switch ((JPH::BroadPhaseLayer::Type)inLayer)
            {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            default:
                return "INVALID";
            }
        }
#endif

    private:
        JPH::BroadPhaseLayer m_objectToBroadPhase[Layers::NUM_LAYERS];
    };

    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                return false;
            }
        }
    };

    BPLayerInterfaceImpl s_broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl s_objectVsBroadPhaseLayerFilter;
    ObjectLayerPairFilterImpl s_objectLayerPairFilter;

} // anonymous namespace

namespace Fermion::Physics3DInternal
{
    JPH::ObjectLayerPairFilter& GetObjectLayerPairFilter()
    {
        return s_objectLayerPairFilter;
    }

    JPH::BroadPhaseLayerInterface& GetBroadPhaseLayerInterface()
    {
        return s_broadPhaseLayerInterface;
    }

    JPH::ObjectVsBroadPhaseLayerFilter& GetObjectVsBroadPhaseLayerFilter()
    {
        return s_objectVsBroadPhaseLayerFilter;
    }

} // namespace Fermion::Physics3DInternal
