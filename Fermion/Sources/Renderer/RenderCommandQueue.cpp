#include "Renderer/RenderCommandQueue.hpp"

namespace Fermion {
void RenderCommandQueue::submit(const std::shared_ptr<CommandBuffer> &buffer) {
    if (!buffer || buffer->empty())
        return;
    m_PendingBuffers.emplace_back(buffer);
}

void RenderCommandQueue::submit(CommandBuffer &&buffer) {
    if (buffer.empty())
        return;

    auto ownedBuffer = std::make_shared<CommandBuffer>(std::move(buffer));
    m_PendingBuffers.emplace_back(std::move(ownedBuffer));
}

void RenderCommandQueue::flush(RendererBackend &backend) {
    for (auto &buffer : m_PendingBuffers) {
        if (!buffer)
            continue;

        buffer->execute(backend);
        buffer->clear();
    }

    m_PendingBuffers.clear();
}

void RenderCommandQueue::clear() {
    m_PendingBuffers.clear();
}

} // namespace Fermion
