#include "OpenGLShader.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "Renderer/Shader.hpp"

namespace Fermion {

std::shared_ptr<Shader> Shader::create(const std::string& name,
                                       const std::string& vertexSrc,
                                       const std::string& fragmentSrc)
{
    switch (Renderer::getAPI()) {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan shader creation is not implemented yet.");
            return nullptr;
    }
    return nullptr;
}

std::shared_ptr<Shader> Shader::create(const std::string& filepath)
{
    switch (Renderer::getAPI()) {
        case RendererAPI::API::None:
            return nullptr;
        case RendererAPI::API::OpenGL:
            return std::make_shared<OpenGLShader>(filepath);
        case RendererAPI::API::Vulkan:
            FERMION_ASSERT(false, "Vulkan shader creation is not implemented yet.");
            return nullptr;
    }
    return nullptr;
}

void ShaderLibrary::add(const std::string& name, const std::shared_ptr<Shader>& shader)
{
    FERMION_ASSERT(!exists(name), "Shader already exists!");
    m_Shaders[name] = shader;
    Log::Info(std::format("Shader loaded: {}", name));
}

void ShaderLibrary::add(const std::shared_ptr<Shader>& shader)
{
    auto& name = shader->getName();
    add(name, shader);
}

std::shared_ptr<Shader> ShaderLibrary::load(const std::string& filepath)
{
    auto shader = Shader::create(filepath);
    add(shader);
    return shader;
}

std::shared_ptr<Shader> ShaderLibrary::load(const std::string& name, const std::string& filepath)
{
    auto shader = Shader::create(filepath);
    add(name, shader);
    return shader;
}

std::shared_ptr<Shader> ShaderLibrary::get(const std::string& name) const
{
    FERMION_ASSERT(exists(name), "Shader not found!");
    return m_Shaders.at(name);
}

bool ShaderLibrary::exists(const std::string& name) const
{
    return m_Shaders.find(name) != m_Shaders.end();
}
} // namespace Fermion
