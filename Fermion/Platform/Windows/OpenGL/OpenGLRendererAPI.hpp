/*
    OpenGLRendererAPI.hpp - OpenGL 渲染接口类
    本头文件定义了 OpenGL 渲染接口类，继承自 Engine 中 RendererAPI 类。
    用于提供 OpenGL 渲染接口。
*/
#pragma once
#include "Renderer/RendererAPI.hpp"

namespace Fermion {

class OpenGLRendererAPI : public RendererAPI {
public:
    virtual void init() override;
    virtual void setViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    virtual void setClearColor(const glm::vec4 &color) override;
    virtual void clear() override;

    virtual void drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) override;
    virtual void drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount, uint32_t indexOffset) override;
    virtual void drawIndexedInstanced(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount, uint32_t instanceCount) override;
    virtual void drawLines(const std::shared_ptr<VertexArray> &vertexArray, uint32_t vertexCount) override;

    virtual void setLineWidth(float width) override;
};

} // namespace Fermion
