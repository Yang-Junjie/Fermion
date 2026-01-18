#include "GraphicsContext.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "OpenGLContext.hpp"
namespace Fermion {
std::unique_ptr<GraphicsContext> GraphicsContext::create(void *window) {
    switch (Renderer::getAPI()) {
    case RendererAPI::API::None:

        return nullptr;
    case RendererAPI::API::OpenGL:
        return std::make_unique<OpenGLContext>(static_cast<GLFWwindow *>(window));
    }
    return nullptr;
}
} // namespace Fermion