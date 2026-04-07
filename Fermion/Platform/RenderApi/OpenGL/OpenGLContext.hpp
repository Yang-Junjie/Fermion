#pragma once

#include "Renderer/GraphicsContext.hpp"

struct GLFWwindow;

namespace Fermion {

class OpenGLContext : public GraphicsContext {
public:
    explicit OpenGLContext(void *windowHandle);

    virtual void init() override;
    virtual void present() override;
    virtual void setVSync(bool enabled) override;

private:
    GLFWwindow *m_windowHandle;
};

} // namespace Fermion
