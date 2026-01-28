#include "GBufferRenderer.hpp"
#include "EnvironmentRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/UniformBufferLayout.hpp"

namespace Fermion
{
    GBufferRenderer::GBufferRenderer()
    {
        // G-Buffer Mesh Pipeline (Phong)
        {
            PipelineSpecification gbufferSpec;
            gbufferSpec.shader = Renderer::getShaderLibrary()->get("GBufferMesh");
            gbufferSpec.depthTest = true;
            gbufferSpec.depthWrite = true;
            gbufferSpec.depthOperator = DepthCompareOperator::Less;
            gbufferSpec.cull = CullMode::Back;

            m_phongPipeline = Pipeline::create(gbufferSpec);
        }

        // G-Buffer Mesh Pipeline (PBR)
        {
            PipelineSpecification gbufferPbrSpec;
            gbufferPbrSpec.shader = Renderer::getShaderLibrary()->get("GBufferPBRMesh");
            gbufferPbrSpec.depthTest = true;
            gbufferPbrSpec.depthWrite = true;
            gbufferPbrSpec.depthOperator = DepthCompareOperator::Less;
            gbufferPbrSpec.cull = CullMode::Back;

            m_pbrPipeline = Pipeline::create(gbufferPbrSpec);
        }
    }

    void GBufferRenderer::ensureFramebuffer(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (m_framebuffer &&
            m_framebuffer->getSpecification().width == width &&
            m_framebuffer->getSpecification().height == height)
        {
            return;
        }

        FramebufferSpecification gBufferSpec;
        gBufferSpec.width = width;
        gBufferSpec.height = height;
        gBufferSpec.attachments = {
            FramebufferTextureFormat::RGBA8,       // Albedo
            FramebufferTextureFormat::RGB16F,      // Normal
            FramebufferTextureFormat::RGB16F,      // Material (Roughness/Metallic/AO)
            FramebufferTextureFormat::RGB16F,      // Emissive
            FramebufferTextureFormat::RED_INTEGER, // ObjectID
            FramebufferTextureFormat::Depth        // Depth
        };
        gBufferSpec.swapChainTarget = false;

        m_framebuffer = Framebuffer::create(gBufferSpec);
    }

    void GBufferRenderer::addPass(RenderGraphLegacy& renderGraph,
                                   const RenderContext& context,
                                   const std::vector<MeshDrawCommand>& drawList,
                                   const std::shared_ptr<Pipeline>& forwardPbrPipeline,
                                   EnvironmentRenderer* environmentRenderer,
                                   ResourceHandle gBuffer,
                                   ResourceHandle sceneDepth,
                                   uint32_t* geometryDrawCalls,
                                   uint32_t* iblDrawCalls)
    {
        LegacyRenderGraphPass pass;
        pass.Name = "GBufferPass";
        pass.Outputs = {gBuffer, sceneDepth};
        pass.Execute = [this, &context, &drawList, forwardPbrPipeline, environmentRenderer, geometryDrawCalls, iblDrawCalls](CommandBuffer& commandBuffer)
        {
            commandBuffer.record([this, &context, &drawList, forwardPbrPipeline, environmentRenderer, geometryDrawCalls, iblDrawCalls](RendererAPI& api)
            {
                if (!m_framebuffer)
                    return;

                m_framebuffer->bind();
                RenderCommand::setBlendEnabled(false);
                RenderCommand::setClearColor({0.0f, 0.0f, 0.0f, 1.0f});
                RenderCommand::clear();
                m_framebuffer->clearAttachment(static_cast<uint32_t>(Attachment::ObjectID), -1);

                std::shared_ptr<Pipeline> currentPipeline = nullptr;

                EnvironmentRenderer::IBLSettings iblSettings = {
                    .useIBL = context.useIBL,
                    .irradianceMapSize = context.irradianceMapSize,
                    .prefilterMapSize = context.prefilterMapSize,
                    .brdfLUTSize = context.brdfLUTSize,
                    .prefilterMaxMipLevels = context.prefilterMaxMipLevels
                };

                if (environmentRenderer)
                {
                    environmentRenderer->ensureIBLInitialized(iblSettings, context.targetFramebuffer,
                                                              context.viewportWidth, context.viewportHeight,
                                                              iblDrawCalls);
                }

                for (const auto& cmd : drawList)
                {
                    if (!cmd.visible || cmd.transparent)
                        continue;

                    const bool isPbr = cmd.pipeline == forwardPbrPipeline;
                    auto desiredPipeline = isPbr ? m_pbrPipeline : m_phongPipeline;
                    if (!desiredPipeline)
                        continue;

                    if (currentPipeline != desiredPipeline)
                    {
                        currentPipeline = desiredPipeline;
                        currentPipeline->bind();
                        auto shader = currentPipeline->getShader();
                        // Camera UBO is already bound globally
                        if (isPbr)
                        {
                            shader->setFloat("u_NormalStrength", context.normalMapStrength);
                            shader->setFloat("u_ToksvigStrength", context.toksvigStrength);
                        }
                    }

                    // Update model uniform buffer for this draw call
                    ModelData modelData;
                    modelData.model = cmd.transform;
                    modelData.normalMatrix = glm::transpose(glm::inverse(cmd.transform));
                    modelData.objectID = cmd.objectID;
                    context.modelUBO->setData(&modelData, sizeof(ModelData));

                    if (cmd.material)
                        cmd.material->bind(currentPipeline->getShader());

                    RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                    if (geometryDrawCalls)
                        (*geometryDrawCalls)++;
                }

                if (context.targetFramebuffer)
                {
                    Framebuffer::blit(m_framebuffer, context.targetFramebuffer, {
                        .mask = FramebufferBlitMask::Depth
                    });
                    context.targetFramebuffer->bind();
                }
                else
                {
                    m_framebuffer->unbind();
                    if (context.viewportWidth > 0 && context.viewportHeight > 0)
                        RenderCommand::setViewport(0, 0, context.viewportWidth, context.viewportHeight);
                    RenderCommand::setBlendEnabled(true);
                }
            });
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
