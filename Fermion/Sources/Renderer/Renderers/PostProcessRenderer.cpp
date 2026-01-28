#include "PostProcessRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "SSGIRenderer.hpp"
#include "GTAORenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommand.hpp"

namespace Fermion
{
    PostProcessRenderer::PostProcessRenderer()
    {
        // DepthView Pipeline
        {
            PipelineSpecification depthViewSpec;
            depthViewSpec.shader = Renderer::getShaderLibrary()->get("DepthView");
            depthViewSpec.depthTest = false;
            depthViewSpec.depthWrite = false;
            depthViewSpec.cull = CullMode::None;
            m_depthViewPipeline = Pipeline::create(depthViewSpec);
        }

        // G-Buffer Debug Pipeline
        {
            PipelineSpecification debugSpec;
            debugSpec.shader = Renderer::getShaderLibrary()->get("GBufferDebug");
            debugSpec.depthTest = false;
            debugSpec.depthWrite = false;
            debugSpec.cull = CullMode::None;

            m_debugPipeline = Pipeline::create(debugSpec);
        }

        // Fullscreen quad
        float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

        uint32_t quadIndices[] = {0, 1, 2, 2, 3, 0};

        auto quadVB = VertexBuffer::create(quadVertices, sizeof(quadVertices));
        quadVB->setLayout({{ShaderDataType::Float3, "a_Position"},
                           {ShaderDataType::Float2, "a_TexCoords"}});

        auto quadIB = IndexBuffer::create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));

        m_quadVA = VertexArray::create();
        m_quadVA->addVertexBuffer(quadVB);
        m_quadVA->setIndexBuffer(quadIB);
    }

    void PostProcessRenderer::addDepthViewPass(RenderGraphLegacy& renderGraph,
                                                const RenderContext& context,
                                                const GBufferRenderer* gBuffer,
                                                bool useDeferred,
                                                float power,
                                                ResourceHandle sceneDepth,
                                                ResourceHandle lightingResult)
    {
        if (!context.targetFramebuffer)
            return;

        LegacyRenderGraphPass pass;
        pass.Name = "DepthViewPass";
        pass.Inputs = {sceneDepth, lightingResult};
        pass.Execute = [this, &context, gBuffer, useDeferred, power](CommandBuffer& commandBuffer)
        {
            commandBuffer.record([this, &context, gBuffer, useDeferred, power](RendererAPI& api)
            {
                m_depthViewPipeline->bind();
                auto shader = m_depthViewPipeline->getShader();

                shader->setInt("u_Depth", 0);
                if (gBuffer && gBuffer->getFramebuffer() && useDeferred)
                {
                    gBuffer->getFramebuffer()->bindDepthAttachment(0);
                }
                else
                {
                    context.targetFramebuffer->bindDepthAttachment(0);
                }

                shader->setFloat("u_Near", context.camera.nearClip);
                shader->setFloat("u_Far", context.camera.farClip);
                shader->setInt("u_IsPerspective", 1);

                shader->setFloat("u_Power", power);

                RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
            });
        };
        renderGraph.addPass(pass);
    }

    void PostProcessRenderer::addGBufferDebugPass(RenderGraphLegacy& renderGraph,
                                                   const RenderContext& context,
                                                   const GBufferRenderer& gBuffer,
                                                   const SSGIRenderer* ssgi,
                                                   const GTAORenderer* gtao,
                                                   GBufferDebugMode mode,
                                                   float depthPower,
                                                   ResourceHandle gBufferHandle,
                                                   ResourceHandle sceneDepth,
                                                   ResourceHandle ssgiHandle,
                                                   ResourceHandle gtaoHandle)
    {
        LegacyRenderGraphPass pass;
        pass.Name = "GBufferDebugPass";
        pass.Inputs = {gBufferHandle, sceneDepth, ssgiHandle, gtaoHandle};
        pass.Execute = [this, &context, &gBuffer, ssgi, gtao, mode, depthPower](CommandBuffer& commandBuffer)
        {
            commandBuffer.record([this, &context, &gBuffer, ssgi, gtao, mode, depthPower](RendererAPI& api)
            {
                auto gBufferFramebuffer = gBuffer.getFramebuffer();
                if (!gBufferFramebuffer || !m_debugPipeline)
                    return;

                if (context.targetFramebuffer)
                {
                    context.targetFramebuffer->bind();
                }
                else
                {
                    if (context.viewportWidth > 0 && context.viewportHeight > 0)
                        RenderCommand::setViewport(0, 0, context.viewportWidth, context.viewportHeight);
                }

                m_debugPipeline->bind();
                auto shader = m_debugPipeline->getShader();

                shader->setInt("u_GBufferAlbedo", 0);
                shader->setInt("u_GBufferNormal", 1);
                shader->setInt("u_GBufferMaterial", 2);
                shader->setInt("u_GBufferEmissive", 3);
                shader->setInt("u_GBufferObjectID", 4);
                shader->setInt("u_GBufferDepth", 5);
                shader->setInt("u_SSGI", 6);
                shader->setInt("u_GTAO", 7);
                shader->setInt("u_Mode", static_cast<int>(mode));
                shader->setFloat("u_Near", context.camera.nearClip);
                shader->setFloat("u_Far", context.camera.farClip);
                shader->setFloat("u_DepthPower", depthPower);

                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Albedo), 0);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal), 1);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Material), 2);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Emissive), 3);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::ObjectID), 4);
                gBufferFramebuffer->bindDepthAttachment(5);
                if (ssgi && ssgi->getResultFramebuffer())
                    ssgi->getResultFramebuffer()->bindColorAttachment(0, 6);
                if (gtao && gtao->getResultFramebuffer())
                    gtao->getResultFramebuffer()->bindColorAttachment(0, 7);

                RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
            });
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
