#pragma once

#include "Renderer/RendererBackend.hpp"

namespace Fermion
{
    /**
     * 记录 CPU 侧渲染命令，之后由 RenderCommandQueue 统一提交给后端。
     */
    class CommandBuffer
    {
    public:
        void record(const RenderCommandData &command);
        void record(const std::function<void(RendererAPI &)> &command);
        void execute(RendererBackend &backend) const;
        void clear();

        bool empty() const
        {
            return m_Commands.empty();
        }
        size_t getCommandCount() const
        {
            return m_Commands.size();
        }

    private:
        std::vector<RenderCommandData> m_Commands;
    };

} // namespace Fermion
