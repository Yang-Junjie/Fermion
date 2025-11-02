﻿#include "Renderer/Texture.hpp"
#include "Renderer/Renderer.hpp"
#include "OpenGLTexture.hpp"
namespace Fermion
{
    std::shared_ptr<Texture2D> Texture2D::create(const std::string &path)
    {
        switch (Renderer::getAPI())
        {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLTexture2D>(path);
        }
        return nullptr;
    }

}
