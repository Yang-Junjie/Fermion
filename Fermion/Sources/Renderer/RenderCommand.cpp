#include "fmpch.hpp"
#include "Renderer/RenderCommand.hpp"

namespace Fermion {

std::unique_ptr<RendererAPI> RenderCommand::s_rendererAPI = RendererAPI::create();

}