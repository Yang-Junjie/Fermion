#include "fmpch.hpp"
#include "Renderer/VertexArray.hpp"

#include "Renderer/RendererAPI.hpp"
#include "OpenGLVertexArray.hpp"

namespace Fermion {

	std::shared_ptr<VertexArray> VertexArray::create()
	{
		switch (RendererAPI::getAPI())
		{
			case RendererAPI::API::None:    return nullptr;
			case RendererAPI::API::OpenGL:  return std::make_shared<OpenGLVertexArray>();
		}
		return nullptr;
	}

}