#include "fmpch.hpp"
#include "Renderer/RendererBackend.hpp"

namespace Fermion
{
    RendererBackend::RendererBackend(RendererAPI &api)
        : m_API(api)
    {
    }

    void RendererBackend::Submit(const RenderCommandData &command)
    {
        if (command.Execute)
            command.Execute(m_API);
    }

}
