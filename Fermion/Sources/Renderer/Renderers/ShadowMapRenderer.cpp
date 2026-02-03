#include "fmpch.hpp"
#include "ShadowMapRenderer.hpp"

#include "Renderer/RenderCommands.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "Renderer/UniformBufferLayout.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/UniformBuffer.hpp"

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

    void ShadowMapRenderer::addPass(RenderGraphLegacy &renderGraph,
                                    ResourceHandle shadowMap,
                                    const std::vector<MeshDrawCommand> &drawList,
                                    const DirectionalLight &light,
                                    uint32_t shadowMapSize,
                                    const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                    uint32_t viewportWidth,
                                    uint32_t viewportHeight,
                                    uint32_t *shadowDrawCalls,
                                    const std::shared_ptr<UniformBuffer> &modelUniformBuffer,
                                    const std::shared_ptr<UniformBuffer> &lightUniformBuffer)
    {
        ensureFramebuffer(shadowMapSize);
        m_lightSpaceMatrix = calculateLightSpaceMatrix(light);

        LegacyRenderGraphPass pass;
        pass.Name = "ShadowPass";
        pass.Outputs = {shadowMap};
        pass.Execute = [this, &drawList, targetFramebuffer, viewportWidth, viewportHeight, shadowDrawCalls, modelUniformBuffer, lightUniformBuffer](RenderCommandQueue& queue)
        {
            queue.submit(CmdBindFramebuffer{m_shadowMapFB});
            queue.submit(CmdClear{});
            queue.submit(CmdBindPipeline{m_shadowPipeline});

            for (auto &cmd : drawList) {
                // Update model uniform buffer for this draw call
                ModelData modelData;
                modelData.model = cmd.transform;
                modelData.normalMatrix = glm::transpose(glm::inverse(cmd.transform));
                modelData.objectID = cmd.objectID;

                queue.submit(CmdCustom{[modelUniformBuffer, modelData]() {
                    modelUniformBuffer->setData(&modelData, sizeof(ModelData));
                }});

                queue.submit(CmdDrawIndexed{cmd.vao, cmd.indexCount, cmd.indexOffset});
                if (shadowDrawCalls)
                    (*shadowDrawCalls)++;
            }

            if (targetFramebuffer) {
                queue.submit(CmdBindFramebuffer{targetFramebuffer});
            } else {
                queue.submit(CmdUnbindFramebuffer{m_shadowMapFB});
                if (viewportWidth > 0 && viewportHeight > 0)
                    queue.submit(CmdSetViewport{0, 0, viewportWidth, viewportHeight});
            }
        };
        renderGraph.addPass(pass);
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
