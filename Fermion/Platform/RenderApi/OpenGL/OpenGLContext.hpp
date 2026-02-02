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