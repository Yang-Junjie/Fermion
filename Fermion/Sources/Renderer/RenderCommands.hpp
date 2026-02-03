#pragma once
#include <cstdint>
#include <variant>
#include <memory>
#include <functional>
#include <glm/glm.hpp>

namespace Fermion {

class Pipeline;
class Framebuffer;
class VertexArray;

// 状态命令

struct CmdSetViewport {
    uint32_t x, y, width, height;
};

struct CmdSetClearColor {
    glm::vec4 color;
};

struct CmdClear {};

struct CmdSetBlendEnabled {
    bool enabled;
};

struct CmdSetLineWidth {
    float width;
};


// 绑定命令
struct CmdBindPipeline {
    std::shared_ptr<Pipeline> pipeline;
};

struct CmdBindFramebuffer {
    std::shared_ptr<Framebuffer> framebuffer;
};

struct CmdUnbindFramebuffer {
    std::shared_ptr<Framebuffer> framebuffer;
};

// 绘制命令

struct CmdDrawIndexed {
    std::shared_ptr<VertexArray> vao;
    uint32_t indexCount;
    uint32_t indexOffset = 0;
};

struct CmdDrawIndexedInstanced {
    std::shared_ptr<VertexArray> vao;
    uint32_t indexCount;
    uint32_t instanceCount;
};

struct CmdDrawLines {
    std::shared_ptr<VertexArray> vao;
    uint32_t vertexCount;
};

// 自定义命令 - 用于复杂操作
// 这是一个过渡方案，未来可以进一步拆分

struct CmdCustom {
    std::function<void()> execute;
};

// 命令变体类型

using RenderCmd = std::variant<
    CmdSetViewport,
    CmdSetClearColor,
    CmdClear,
    CmdSetBlendEnabled,
    CmdSetLineWidth,
    CmdBindPipeline,
    CmdBindFramebuffer,
    CmdUnbindFramebuffer,
    CmdDrawIndexed,
    CmdDrawIndexedInstanced,
    CmdDrawLines,
    CmdCustom
>;

} // namespace Fermion
