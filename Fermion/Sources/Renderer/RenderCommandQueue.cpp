#include "Renderer/RenderCommandQueue.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/VertexArray.hpp"

namespace Fermion {

namespace {
    // 命令执行器 - 使用 overloaded 模式实现 visitor
    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
}

void RenderCommandQueue::flush(RendererAPI& api) {
    // 复制命令列表以便释放锁
    std::vector<RenderCmd> commands;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        commands = std::move(m_Commands);
        m_Commands.clear();
    }

    // 执行所有命令
    for (auto& cmd : commands) {
        std::visit(overloaded{
            [&api](const CmdSetViewport& c) {
                api.setViewport(c.x, c.y, c.width, c.height);
            },
            [&api](const CmdSetClearColor& c) {
                api.setClearColor(c.color);
            },
            [&api](const CmdClear&) {
                api.clear();
            },
            [&api](const CmdSetBlendEnabled& c) {
                api.setBlendEnabled(c.enabled);
            },
            [&api](const CmdSetLineWidth& c) {
                api.setLineWidth(c.width);
            },
            [](const CmdBindPipeline& c) {
                if (c.pipeline)
                    c.pipeline->bind();
            },
            [](const CmdBindFramebuffer& c) {
                if (c.framebuffer)
                    c.framebuffer->bind();
            },
            [](const CmdUnbindFramebuffer& c) {
                if (c.framebuffer)
                    c.framebuffer->unbind();
            },
            [&api](const CmdDrawIndexed& c) {
                if (c.vao)
                    api.drawIndexed(c.vao, c.indexCount, c.indexOffset);
            },
            [&api](const CmdDrawIndexedInstanced& c) {
                if (c.vao)
                    api.drawIndexedInstanced(c.vao, c.indexCount, c.instanceCount);
            },
            [&api](const CmdDrawLines& c) {
                if (c.vao)
                    api.drawLines(c.vao, c.vertexCount);
            },
            [](const CmdCustom& c) {
                if (c.execute)
                    c.execute();
            }
        }, cmd);
    }
}

void RenderCommandQueue::clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Commands.clear();
}

} // namespace Fermion
