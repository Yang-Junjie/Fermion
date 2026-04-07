#include "PostProcessRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/TextureBinding.hpp"
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

    void PostProcessRenderer::addDepthViewPass(RenderPassQueue& passQueue,
                                                const RenderContext& context,
                                                const GBufferRenderer* gBuffer,
                                                bool useDeferred,
                                                float power,
                                                ResourceHandle sceneDepth,
                                                ResourceHandle lightingResult)
    {
        if (!context.targetFramebuffer)
            return;

        RenderPass pass;
        pass.Name = "DepthViewPass";
        pass.Inputs = {sceneDepth, lightingResult};
        pass.Execute = [this, &context, gBuffer, useDeferred, power](RendererAPI& api)
        {
            auto gBufferFB = (gBuffer && useDeferred) ? gBuffer->getFramebuffer() : nullptr;
            auto targetFB = context.targetFramebuffer;
            float nearClip = context.camera.nearClip;
            float farClip = context.camera.farClip;

            m_depthViewPipeline->bind();
            auto shader = m_depthViewPipeline->getShader();

            if (gBufferFB && useDeferred)
                gBufferFB->bindDepthAttachment(TextureBinding::PostProcess::Depth);
            else
                targetFB->bindDepthAttachment(TextureBinding::PostProcess::Depth);

            shader->setFloat("u_Near", nearClip);
            shader->setFloat("u_Far", farClip);
            shader->setInt("u_IsPerspective", 1);
            shader->setFloat("u_Power", power);

            api.drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
        };
        passQueue.addPass(pass);
    }

    void PostProcessRenderer::addGBufferDebugPass(RenderPassQueue& passQueue,
                                                   const RenderContext& context,
                                                   const GBufferRenderer& gBuffer,
                                                   GBufferDebugMode mode,
                                                   float depthPower,
                                                   ResourceHandle gBufferHandle,
                                                   ResourceHandle sceneDepth)
    {
        RenderPass pass;
        pass.Name = "GBufferDebugPass";
        pass.Inputs = {gBufferHandle, sceneDepth};
        pass.Execute = [this, &context, &gBuffer, mode, depthPower](RendererAPI& api)
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
                    api.setViewport(0, 0, context.viewportWidth, context.viewportHeight);
            }

            float nearClip = context.camera.nearClip;
            float farClip = context.camera.farClip;

            m_debugPipeline->bind();
            auto shader = m_debugPipeline->getShader();

            shader->setInt("u_Mode", static_cast<int>(mode));
            shader->setFloat("u_Near", nearClip);
            shader->setFloat("u_Far", farClip);
            shader->setFloat("u_DepthPower", depthPower);

            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Albedo),
                                                    TextureBinding::GBufferDebug::Albedo);
            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal),
                                                    TextureBinding::GBufferDebug::Normal);
            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Material),
                                                    TextureBinding::GBufferDebug::Material);
            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Emissive),
                                                    TextureBinding::GBufferDebug::Emissive);
            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::ObjectID),
                                                    TextureBinding::GBufferDebug::ObjectID);
            gBufferFramebuffer->bindDepthAttachment(TextureBinding::GBufferDebug::Depth);

            api.drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
        };
        passQueue.addPass(pass);
    }

} // namespace Fermion
