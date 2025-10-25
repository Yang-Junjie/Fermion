#include "GraphicsContext.hpp"
#include "Renderer/RendererAPI.hpp"
#include "OpenGLContext.hpp"
namespace Fermion
{
    std::unique_ptr<GraphicsContext> GraphicsContext::create(void *window)
    {
        switch (RendererAPI::getAPI())
        {
        case RendererAPI::API::None:
           
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLContext>(static_cast<GLFWwindow *>(window));
        }
        return nullptr;
    }
}