#include "fmpch.hpp"
#include "Renderer/RendererBackend.hpp"

namespace Fermion {
RendererBackend::RendererBackend(RendererAPI &api) : m_API(api) {
}

void RendererBackend::submit(const RenderCommandData &command) {
    if (command.execute)
        command.execute(m_API);
}

} // namespace Fermion
