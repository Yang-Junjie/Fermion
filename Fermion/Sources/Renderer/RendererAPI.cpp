#include "fmpch.hpp"
#include "Renderer/RendererAPI.hpp"

#include "OpenGLRendererAPI.hpp"

namespace Fermion {

RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

std::unique_ptr<RendererAPI> RendererAPI::create() {
    switch (s_API) {
    case RendererAPI::API::None:
        return nullptr;
    case RendererAPI::API::OpenGL:
        return std::make_unique<OpenGLRendererAPI>();
    }
    return nullptr;
}

} // namespace Fermion