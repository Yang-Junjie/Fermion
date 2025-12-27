/*
    OpenGLContext.hpp - OpenGL 图形上下文实现

    本文件定义了 OpenGLContext 类，继承自 GraphicsContext 抽象基类，
    用于管理 OpenGL 上下文的初始化和缓冲区交换等操作
*/

#pragma once

#include "Renderer/GraphicsContext.hpp"

struct GLFWwindow;

namespace Fermion {

class OpenGLContext : public GraphicsContext {
public:
    OpenGLContext(GLFWwindow *windowHandle);

    virtual void init() override;
    virtual void swapBuffers() override;

private:
    GLFWwindow *m_windowHandle;
};

} // namespace Fermion