#include "Renderer/Shader.hpp"
#include "OpenGLShader.hpp"
#include "Renderer/Renderer.hpp"
namespace Fermion
{
    std::shared_ptr<Shader> Shader::create(const std::string &vertexSrc, const std::string &fragmentSrc){
        switch (Renderer::getAPI())
		{
			case RendererAPI::API::None:    return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLShader>(vertexSrc, fragmentSrc);
		}
		return nullptr;
    }

	std::shared_ptr<Shader> Shader::create(const std::string &filepath){
		switch (Renderer::getAPI())
		{
			case RendererAPI::API::None:    return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLShader>(filepath);
		}
		return nullptr;
	}
}
