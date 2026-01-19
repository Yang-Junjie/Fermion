#pragma once

#include "Renderer/RendererAPI.hpp"

namespace Fermion {
struct RenderCommandData {
    std::function<void(RendererAPI &)> execute;
};

/**
 * 统一的渲染后端入口，封装了底层 RendererAPI。
 * 目前 Submit 会立即调用底层 API，后续可以扩展为更复杂的命令调度。
 */
class RendererBackend {
public:
    explicit RendererBackend(RendererAPI &api);

    RendererAPI &getAPI() const {
        return m_API;
    }

    void submit(const RenderCommandData &command);

private:
    RendererAPI &m_API;
};

} // namespace Fermion
