
#include "Renderer/Renderer.hpp"

namespace Fermion
{
    std::unique_ptr<Renderer::SceneData> Renderer::s_sceneData = std::make_unique<Renderer::SceneData>();
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

    void Renderer::beginScene(OrthographicCamera &camera)
    {
        s_sceneData->viewProjectionMatrix = camera.getViewProjectionMatrix();
    }

    void Renderer::endScene()
    {
    }

    void Renderer::submit(const std::shared_ptr<OpenGLShader> &shader, const std::shared_ptr<VertexArray> &vertexArray, const glm::mat4 &transform)
    {
        shader->bind();
        shader->setMat4("u_ViewProjection", s_sceneData->viewProjectionMatrix);
        shader->setMat4("u_Transform", transform);
        vertexArray->bind();
        RenderCommand::drawIndexed(vertexArray);
    }

}
