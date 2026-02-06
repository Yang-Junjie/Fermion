#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace JPH
{
    class ObjectLayerPairFilter;
    class BroadPhaseLayerInterface;
    class ObjectVsBroadPhaseLayerFilter;
}

namespace Fermion::Physics3DInternal
{
    namespace Layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
    } 

    namespace BroadPhaseLayers
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::uint NUM_LAYERS = 2;
    } 

    JPH::ObjectLayerPairFilter &GetObjectLayerPairFilter();

    JPH::BroadPhaseLayerInterface &GetBroadPhaseLayerInterface();

    JPH::ObjectVsBroadPhaseLayerFilter &GetObjectVsBroadPhaseLayerFilter();

} // namespace Fermion::Physics3DInternal
