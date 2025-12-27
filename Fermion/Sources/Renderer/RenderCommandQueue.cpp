#include "Renderer/RenderCommandQueue.hpp"

namespace Fermion {
void RenderCommandQueue::Submit(const std::shared_ptr<CommandBuffer> &buffer) {
    if (!buffer || buffer->Empty())
        return;
    m_PendingBuffers.emplace_back(buffer);
}

void RenderCommandQueue::Submit(CommandBuffer &&buffer) {
    if (buffer.Empty())
        return;

    auto ownedBuffer = std::make_shared<CommandBuffer>(std::move(buffer));
    m_PendingBuffers.emplace_back(std::move(ownedBuffer));
}

void RenderCommandQueue::Flush(RendererBackend &backend) {
    for (auto &buffer : m_PendingBuffers) {
        if (!buffer)
            continue;

        buffer->Execute(backend);
        buffer->Clear();
    }

    m_PendingBuffers.clear();
}

void RenderCommandQueue::Clear() {
    m_PendingBuffers.clear();
}

} // namespace Fermion
