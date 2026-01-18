#include "fmpch.hpp"
#include "ShadowMapRenderer.hpp"

#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderers/Renderer.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Fermion
{
    ShadowMapRenderer::ShadowMapRenderer()
    {
        PipelineSpecification shadowSpec;
        shadowSpec.shader = Renderer::getShaderLibrary()->get("Shadow");
        shadowSpec.depthTest = true;
        shadowSpec.depthWrite = true;
        shadowSpec.depthOperator = DepthCompareOperator::Less;
        shadowSpec.cull = CullMode::Back;

        m_shadowPipeline = Pipeline::create(shadowSpec);
    }

    void ShadowMapRenderer::addPass(RenderGraph &renderGraph,
                                    ResourceHandle shadowMap,
                                    const std::vector<MeshDrawCommand> &drawList,
                                    const DirectionalLight &light,
                                    uint32_t shadowMapSize,
                                    const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                    uint32_t viewportWidth,
                                    uint32_t viewportHeight,
                                    uint32_t *shadowDrawCalls)
    {
        ensureFramebuffer(shadowMapSize);
        m_lightSpaceMatrix = calculateLightSpaceMatrix(light);

        renderGraph.addPass(
            {.Name = "ShadowPass",
             .Outputs = {shadowMap},
             .Execute = [this, &drawList, targetFramebuffer, viewportWidth, viewportHeight, shadowDrawCalls](CommandBuffer &commandBuffer)
             {
                 commandBuffer.record([this, &drawList, targetFramebuffer, viewportWidth, viewportHeight, shadowDrawCalls](RendererAPI &api)
                                      {
                     m_shadowMapFB->bind();
                     RenderCommand::clear();

                     m_shadowPipeline->bind();
                     auto shader = m_shadowPipeline->getShader();
                     shader->setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);

                     for (auto &cmd : drawList) {
                         shader->setMat4("u_Model", cmd.transform);
                         RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                         if (shadowDrawCalls)
                             (*shadowDrawCalls)++;
                     }

                     if (targetFramebuffer) {
                         targetFramebuffer->bind();
                     } else {
                         m_shadowMapFB->unbind();
                         if (viewportWidth > 0 && viewportHeight > 0)
                             RenderCommand::setViewport(0, 0, viewportWidth, viewportHeight);
                     } });
             }});
    }

    const glm::mat4 &ShadowMapRenderer::getLightSpaceMatrix() const
    {
        return m_lightSpaceMatrix;
    }

    std::shared_ptr<Framebuffer> ShadowMapRenderer::getShadowMapFramebuffer() const
    {
        return m_shadowMapFB;
    }

    void ShadowMapRenderer::ensureFramebuffer(uint32_t size)
    {
        if (!m_shadowMapFB || m_shadowMapFB->getSpecification().width != size)
        {
            FramebufferSpecification shadowFBSpec;
            shadowFBSpec.width = size;
            shadowFBSpec.height = size;
            shadowFBSpec.attachments = {FramebufferTextureFormat::DEPTH_COMPONENT32F};
            shadowFBSpec.swapChainTarget = false;

            m_shadowMapFB = Framebuffer::create(shadowFBSpec);
        }
    }

    glm::mat4 ShadowMapRenderer::calculateLightSpaceMatrix(const DirectionalLight &light, float orthoSize) const
    {
        glm::vec3 lightDir = glm::normalize(light.direction);
        glm::vec3 lightPos = lightDir * orthoSize;

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(lightDir, up)) > 0.99f)
        {
            up = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        glm::mat4 lightView = glm::lookAt(
            lightPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            up);

        glm::mat4 lightProjection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize, orthoSize,
            0.1f, orthoSize * 3.0f);

        return lightProjection * lightView;
    }
} // namespace Fermion
