#pragma once
#include "Renderer/CommandBuffer.hpp"

namespace Fermion {
class RenderCommandQueue {
public:
    void submit(const std::shared_ptr<CommandBuffer> &buffer);
    void submit(CommandBuffer &&buffer);

    void flush(RendererBackend &backend);
    void clear();

    bool empty() const {
        return m_PendingBuffers.empty();
    }
    size_t getPendingBufferCount() const {
        return m_PendingBuffers.size();
    }

private:
    std::vector<std::shared_ptr<CommandBuffer>> m_PendingBuffers;
};

} // namespace Fermion
