#pragma once
#include "Core/Timestep.hpp"

namespace Fermion {

class IImGuiBackend
{
public:
    virtual ~IImGuiBackend() = default;

    virtual bool Init(void* nativeWindow) = 0;
    virtual void BeginFrame(Timestep dt) = 0;
    virtual void EndFrame() = 0;
    virtual void Shutdown() = 0;
};

} 
