#pragma once
#include <memory>
namespace Fermion
{
    class GraphicsContext
    {
    public:
        virtual void init() = 0;
        virtual void swapBuffers() = 0;
        virtual ~GraphicsContext() = default;
        static std::unique_ptr<GraphicsContext> create(void *window);
    };
}