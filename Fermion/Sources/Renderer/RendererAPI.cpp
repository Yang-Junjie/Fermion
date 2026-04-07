#include "fmpch.hpp"
#include "Renderer/RendererAPI.hpp"

#include "Core/Log.hpp"
#include "OpenGLRendererAPI.hpp"

namespace Fermion {

RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

std::unique_ptr<RendererAPI> RendererAPI::create()
{
    switch (s_API) {
    case RendererAPI::API::None:
        return nullptr;
    case RendererAPI::API::OpenGL:
        return std::make_unique<OpenGLRendererAPI>();
    case RendererAPI::API::Vulkan:
        Log::Error("Vulkan renderer backend is not implemented yet.");
        return nullptr;
    }

    return nullptr;
}

void RendererAPI::setAPI(API api)
{
    s_API = api;
}

bool RendererAPI::isBackendImplemented(API api)
{
    switch (api) {
    case API::None:
        return false;
    case API::OpenGL:
        return true;
    case API::Vulkan:
        return false;
    }

    return false;
}

const char* RendererAPI::toString(API api)
{
    switch (api) {
    case API::None:
        return "None";
    case API::OpenGL:
        return "OpenGL";
    case API::Vulkan:
        return "Vulkan";
    }

    return "Unknown";
}

} // namespace Fermion
