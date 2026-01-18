#include "fmpch.hpp"
#include "Renderer/CommandBuffer.hpp"

namespace Fermion {
void CommandBuffer::record(const RenderCommandData &command) {
    if (command.execute)
        m_Commands.emplace_back(command);
}

void CommandBuffer::record(const std::function<void(RendererAPI &)> &command) {
    if (!command)
        return;

    RenderCommandData data;
    data.execute = command;
    record(data);
}

void CommandBuffer::execute(RendererBackend &backend) const {
    for (auto &command : m_Commands)
        backend.submit(command);
}

void CommandBuffer::clear() {
    m_Commands.clear();
}

} // namespace Fermion
