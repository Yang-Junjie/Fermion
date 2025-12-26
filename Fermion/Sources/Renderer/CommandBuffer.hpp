#pragma once

#include "fmpch.hpp"
#include "Renderer/RendererBackend.hpp"

namespace Fermion
{
    /**
     * 记录 CPU 侧渲染命令，之后由 RenderCommandQueue 统一提交给后端。
     */
    class CommandBuffer
    {
    public:
        void Record(const RenderCommandData &command);
        void Record(const std::function<void(RendererAPI &)> &command);
        void Execute(RendererBackend &backend) const;
        void Clear();

        bool Empty() const { return m_Commands.empty(); }
        size_t GetCommandCount() const { return m_Commands.size(); }

    private:
        std::vector<RenderCommandData> m_Commands;
    };

}
