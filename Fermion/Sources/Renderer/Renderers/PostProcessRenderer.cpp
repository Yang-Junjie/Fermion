#include "PostProcessRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommands.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/VertexArray.hpp"

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
        pass.Execute = [this, &context, gBuffer, useDeferred, power](RenderCommandQueue& queue)
        {
            auto gBufferFB = (gBuffer && useDeferred) ? gBuffer->getFramebuffer() : nullptr;
            auto targetFB = context.targetFramebuffer;
            float nearClip = context.camera.nearClip;
            float farClip = context.camera.farClip;

            queue.submit(CmdCustom{[this, gBufferFB, targetFB, useDeferred, power, nearClip, farClip]() {
                m_depthViewPipeline->bind();
                auto shader = m_depthViewPipeline->getShader();

                shader->setInt("u_Depth", 0);
                if (gBufferFB && useDeferred)
                {
                    gBufferFB->bindDepthAttachment(0);
                }
                else
                {
                    targetFB->bindDepthAttachment(0);
                }

                shader->setFloat("u_Near", nearClip);
                shader->setFloat("u_Far", farClip);
                shader->setInt("u_IsPerspective", 1);

                shader->setFloat("u_Power", power);
            }});

            queue.submit(CmdDrawIndexed{m_quadVA, m_quadVA->getIndexBuffer()->getCount()});
        };
        renderGraph.addPass(pass);
    }

    void PostProcessRenderer::addGBufferDebugPass(RenderGraphLegacy& renderGraph,
                                                   const RenderContext& context,
                                                   const GBufferRenderer& gBuffer,
                                                   GBufferDebugMode mode,
                                                   float depthPower,
                                                   ResourceHandle gBufferHandle,
                                                   ResourceHandle sceneDepth)
    {
        LegacyRenderGraphPass pass;
        pass.Name = "GBufferDebugPass";
        pass.Inputs = {gBufferHandle, sceneDepth};
        pass.Execute = [this, &context, &gBuffer, mode, depthPower](RenderCommandQueue& queue)
        {
            auto gBufferFramebuffer = gBuffer.getFramebuffer();
            if (!gBufferFramebuffer || !m_debugPipeline)
                return;

            if (context.targetFramebuffer)
            {
                queue.submit(CmdBindFramebuffer{context.targetFramebuffer});
            }
            else
            {
                if (context.viewportWidth > 0 && context.viewportHeight > 0)
                    queue.submit(CmdSetViewport{0, 0, context.viewportWidth, context.viewportHeight});
            }

            float nearClip = context.camera.nearClip;
            float farClip = context.camera.farClip;

            queue.submit(CmdCustom{[this, gBufferFramebuffer,
                                    mode, nearClip, farClip, depthPower]() {
                m_debugPipeline->bind();
                auto shader = m_debugPipeline->getShader();

                shader->setInt("u_GBufferAlbedo", 0);
                shader->setInt("u_GBufferNormal", 1);
                shader->setInt("u_GBufferMaterial", 2);
                shader->setInt("u_GBufferEmissive", 3);
                shader->setInt("u_GBufferObjectID", 4);
                shader->setInt("u_GBufferDepth", 5);
                shader->setInt("u_Mode", static_cast<int>(mode));
                shader->setFloat("u_Near", nearClip);
                shader->setFloat("u_Far", farClip);
                shader->setFloat("u_DepthPower", depthPower);

                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Albedo), 0);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal), 1);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Material), 2);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Emissive), 3);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::ObjectID), 4);
                gBufferFramebuffer->bindDepthAttachment(5);
            }});

            queue.submit(CmdDrawIndexed{m_quadVA, m_quadVA->getIndexBuffer()->getCount()});
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
