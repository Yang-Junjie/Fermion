#include "Renderer/Renderers/Renderer.hpp"
#include "OpenGLTexture.hpp"
#include "Texture.hpp"
namespace Fermion
{
    std::unique_ptr<Texture2D> Texture2D::create(const std::string &path, bool generateMips)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLTexture2D>(path, generateMips);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan 2D texture creation is not implemented yet.");
            return nullptr;
        }
        return nullptr;
    }

    std::unique_ptr<Texture2D> Texture2D::create(const TextureSpecification &spec)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLTexture2D>(spec, spec.GenerateMips);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan 2D texture creation is not implemented yet.");
            return nullptr;
        }
        return nullptr;
    }

    std::unique_ptr<Texture2D> Texture2D::create(const TextureAssetSpecification &assetSpec)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLTexture2D>(assetSpec);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan 2D texture creation is not implemented yet.");
            return nullptr;
        }
        return nullptr;
    }

    std::unique_ptr<Texture2D> Texture2D::create(uint32_t width, uint32_t height, bool generateMips)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLTexture2D>(width, height, generateMips);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan 2D texture creation is not implemented yet.");
            return nullptr;
        }
        return nullptr;
    }
    std::unique_ptr<TextureCube> TextureCube::create(const std::string &path)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLTextureCube>(path);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan cube texture creation is not implemented yet.");
            return nullptr;
        }
        return nullptr;
    }
    std::unique_ptr<TextureCube> TextureCube::create(const TextureCubeSpecification &spec)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_unique<OpenGLTextureCube>(spec);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan cube texture creation is not implemented yet.");
            return nullptr;
        }
        return nullptr;
    }
}
