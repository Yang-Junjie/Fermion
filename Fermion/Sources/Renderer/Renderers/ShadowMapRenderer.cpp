#include "fmpch.hpp"
#include "ShadowMapRenderer.hpp"
#include "SceneRenderer.hpp"

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

        // Skinned shadow pipeline
        PipelineSpecification skinnedShadowSpec;
        skinnedShadowSpec.shader = Renderer::getShaderLibrary()->get("SkinnedShadow");
        skinnedShadowSpec.depthTest = true;
        skinnedShadowSpec.depthWrite = true;
        skinnedShadowSpec.depthOperator = DepthCompareOperator::Less;
        skinnedShadowSpec.cull = CullMode::Back;

        m_skinnedShadowPipeline = Pipeline::create(skinnedShadowSpec);
    }

    void ShadowMapRenderer::addPass(RenderPassQueue &passQueue,
                                    ResourceHandle shadowMap,
                                    std::span<const MeshDrawCommand> drawList,
                                    const DirectionalLight &light,
                                    uint32_t shadowMapSize,
                                    const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                    uint32_t viewportWidth,
                                    uint32_t viewportHeight,
                                    uint32_t *shadowDrawCalls,
                                    const std::shared_ptr<UniformBuffer> &modelUniformBuffer,
                                    const std::shared_ptr<UniformBuffer> &lightUniformBuffer,
                                    const std::shared_ptr<UniformBuffer> &boneUniformBuffer)
    {
        ensureFramebuffer(shadowMapSize);
        m_lightSpaceMatrix = calculateLightSpaceMatrix(light);

        RenderPass pass;
        pass.Name = "ShadowPass";
        pass.Outputs = {shadowMap};
        pass.Execute = [this, drawList, targetFramebuffer, viewportWidth, viewportHeight, shadowDrawCalls, modelUniformBuffer, lightUniformBuffer, boneUniformBuffer](RendererAPI& api)
        {
            m_shadowMapFB->bind();
            api.clear();

            std::shared_ptr<Pipeline> currentPipeline = nullptr;

            for (auto &cmd : drawList) {
                // Select appropriate pipeline
                auto desiredPipeline = cmd.isSkinned ? m_skinnedShadowPipeline : m_shadowPipeline;
                if (currentPipeline != desiredPipeline)
                {
                    currentPipeline = desiredPipeline;
                    currentPipeline->bind();
                }

                // Update model uniform buffer for this draw call
                ModelData modelData;
                modelData.model = cmd.transform;
                modelData.normalMatrix = glm::transpose(glm::inverse(cmd.transform));
                modelData.objectID = cmd.objectID;

                modelUniformBuffer->setData(&modelData, sizeof(ModelData));

                // Upload bone matrices for skinned meshes
                if (cmd.isSkinned && cmd.boneMatrices && !cmd.boneMatrices->empty() && boneUniformBuffer)
                {
                    boneUniformBuffer->setData(cmd.boneMatrices->data(),
                        static_cast<uint32_t>(cmd.boneMatrices->size() * sizeof(glm::mat4)));
                }

                api.drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                if (shadowDrawCalls)
                    (*shadowDrawCalls)++;
            }

            if (targetFramebuffer) {
                targetFramebuffer->bind();
            } else {
                m_shadowMapFB->unbind();
                if (viewportWidth > 0 && viewportHeight > 0)
                    api.setViewport(0, 0, viewportWidth, viewportHeight);
            }
        };
        passQueue.addPass(pass);
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
