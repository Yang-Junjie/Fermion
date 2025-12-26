#include "fmpch.hpp"
#include "Renderer/CommandBuffer.hpp"

namespace Fermion
{
    void CommandBuffer::Record(const RenderCommandData &command)
    {
        if (command.Execute)
            m_Commands.emplace_back(command);
    }

    void CommandBuffer::Record(const std::function<void(RendererAPI &)> &command)
    {
        if (!command)
            return;

        RenderCommandData data;
        data.Execute = command;
        Record(data);
    }

    void CommandBuffer::Execute(RendererBackend &backend) const
    {
        for (auto &command : m_Commands)
            backend.Submit(command);
    }

    void CommandBuffer::Clear()
    {
        m_Commands.clear();
    }

}
