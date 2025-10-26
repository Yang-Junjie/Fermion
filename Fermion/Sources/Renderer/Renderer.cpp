#include "fmpch.hpp"
#include "Renderer/Renderer.hpp"

namespace Fermion
{
    void Renderer::init()
    {
        RenderCommand::init();
    }

    void Renderer::shutdown()
    {
    }

    void Renderer::onWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::setViewport(0, 0, width, height);
    }

    void Renderer::beginScene()
    {
    }

    void Renderer::endScene()
    {
    }

    void Renderer::submit(const std::shared_ptr<VertexArray> &vertexArray)
    {
        vertexArray->bind();
        RenderCommand::drawIndexed(vertexArray);
    }

}
