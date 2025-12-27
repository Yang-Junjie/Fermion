#pragma once
#include "Renderer/CommandBuffer.hpp"

namespace Fermion {
class RenderCommandQueue {
public:
    void Submit(const std::shared_ptr<CommandBuffer> &buffer);
    void Submit(CommandBuffer &&buffer);

    void Flush(RendererBackend &backend);
    void Clear();

    bool Empty() const {
        return m_PendingBuffers.empty();
    }
    size_t GetPendingBufferCount() const {
        return m_PendingBuffers.size();
    }

private:
    std::vector<std::shared_ptr<CommandBuffer>> m_PendingBuffers;
};

} // namespace Fermion
