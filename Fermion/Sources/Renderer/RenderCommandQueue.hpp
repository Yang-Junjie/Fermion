#pragma once
#include "Renderer/RenderCommands.hpp"
#include <vector>
#include <mutex>

namespace Fermion {

class RendererAPI;

class RenderCommandQueue {
public:
    // 提交单个命令
    template<typename T>
    void submit(T&& cmd) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Commands.emplace_back(std::forward<T>(cmd));
    }

    // 批量提交命令
    template<typename... Args>
    void submitBatch(Args&&... cmds) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        (m_Commands.emplace_back(std::forward<Args>(cmds)), ...);
    }

    // 执行所有命令
    void flush(RendererAPI& api);

    // 清空命令队列
    void clear();

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Commands.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_Mutex);
        return m_Commands.size();
    }

private:
    std::vector<RenderCmd> m_Commands;
    mutable std::mutex m_Mutex;
};

} // namespace Fermion
