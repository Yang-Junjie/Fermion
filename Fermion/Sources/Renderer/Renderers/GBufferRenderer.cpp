#include "GBufferRenderer.hpp"
#include "EnvironmentRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommands.hpp"
#include "Renderer/UniformBufferLayout.hpp"
#include "Renderer/UniformBuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Core/Log.hpp"

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

        // Skinned G-Buffer PBR Pipeline
        {
            PipelineSpecification skinnedGBufferSpec;
            skinnedGBufferSpec.shader = Renderer::getShaderLibrary()->get("SkinnedGBufferPBRMesh");
            skinnedGBufferSpec.depthTest = true;
            skinnedGBufferSpec.depthWrite = true;
            skinnedGBufferSpec.depthOperator = DepthCompareOperator::Less;
            skinnedGBufferSpec.cull = CullMode::Back;

            m_skinnedGBufferPipeline = Pipeline::create(skinnedGBufferSpec);
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
        pass.Execute = [this, &context, &drawList, forwardPbrPipeline, environmentRenderer, geometryDrawCalls, iblDrawCalls](RenderCommandQueue& queue)
        {
            if (!m_framebuffer)
                return;

            queue.submit(CmdBindFramebuffer{m_framebuffer});
            queue.submit(CmdSetClearColor{{0.0f, 0.0f, 0.0f, 1.0f}});
            queue.submit(CmdClear{});
            queue.submit(CmdCustom{[this]() {
                m_framebuffer->clearAttachment(static_cast<uint32_t>(Attachment::ObjectID), -1);
            }});

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

                std::shared_ptr<Pipeline> desiredPipeline;
                bool isPbr;

                if (cmd.isSkinned)
                {
                    desiredPipeline = m_skinnedGBufferPipeline;
                    isPbr = true;
                }
                else
                {
                    isPbr = cmd.pipeline == forwardPbrPipeline;
                    desiredPipeline = isPbr ? m_pbrPipeline : m_phongPipeline;
                }

                if (!desiredPipeline)
                    continue;

                if (currentPipeline != desiredPipeline)
                {
                    currentPipeline = desiredPipeline;
                    queue.submit(CmdCustom{[currentPipeline, isPbr, &context]() {
                        currentPipeline->bind();
                        auto shader = currentPipeline->getShader();
                        // Camera UBO is already bound globally
                        if (isPbr)
                        {
                            shader->setFloat("u_NormalStrength", context.normalMapStrength);
                            shader->setFloat("u_ToksvigStrength", context.toksvigStrength);
                        }
                    }});
                }

                // Update model uniform buffer for this draw call
                ModelData modelData;
                modelData.model = cmd.transform;
                modelData.normalMatrix = glm::transpose(glm::inverse(cmd.transform));
                modelData.objectID = cmd.objectID;

                // Upload bone matrices for skinned meshes
                if (cmd.isSkinned && cmd.boneMatrices && !cmd.boneMatrices->empty())
                {
                    queue.submit(CmdCustom{[boneUBO = context.boneUBO, boneMatrices = cmd.boneMatrices]() {
                        boneUBO->setData(boneMatrices->data(),
                            static_cast<uint32_t>(boneMatrices->size() * sizeof(glm::mat4)));
                    }});
                }

                queue.submit(CmdCustom{[modelUBO = context.modelUBO, modelData, currentPipeline, material = cmd.material]() {
                    modelUBO->setData(&modelData, sizeof(ModelData));
                    if (material)
                        material->bind(currentPipeline->getShader());
                }});

                queue.submit(CmdDrawIndexed{cmd.vao, cmd.indexCount, cmd.indexOffset});
                if (geometryDrawCalls)
                    (*geometryDrawCalls)++;
            }

            if (context.targetFramebuffer)
            {
                queue.submit(CmdCustom{[this, targetFB = context.targetFramebuffer]() {
                    Framebuffer::blit(m_framebuffer, targetFB, {
                        .mask = FramebufferBlitMask::Depth
                    });
                    targetFB->bind();
                }});
            }
            else
            {
                // Blit depth to default framebuffer (0) for correct skybox/transparent rendering
                queue.submit(CmdCustom{[this, vpW = context.viewportWidth, vpH = context.viewportHeight]() {
                    Log::Trace(std::format("[GBuffer] Blitting depth to default framebuffer (viewport: {}x{})", vpW, vpH));
                    Framebuffer::blitToDefault(m_framebuffer, vpW, vpH, {
                        .mask = FramebufferBlitMask::Depth
                    });
                    Log::Trace(std::format("[GBuffer] Blit complete, should be bound to default FB (0)"));
                }});
                if (context.viewportWidth > 0 && context.viewportHeight > 0)
                    queue.submit(CmdSetViewport{0, 0, context.viewportWidth, context.viewportHeight});
            }
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
