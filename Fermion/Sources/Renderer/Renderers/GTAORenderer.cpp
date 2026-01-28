#include "GTAORenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommand.hpp"

namespace Fermion
{
    GTAORenderer::GTAORenderer()
    {
        // GTAO Pipeline
        {
            PipelineSpecification gtaoSpec;
            gtaoSpec.shader = Renderer::getShaderLibrary()->get("GTAO");
            gtaoSpec.depthTest = false;
            gtaoSpec.depthWrite = false;
            gtaoSpec.cull = CullMode::None;

            m_pipeline = Pipeline::create(gtaoSpec);
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

    void GTAORenderer::ensureFramebuffer(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (m_framebuffer &&
            m_framebuffer->getSpecification().width == width &&
            m_framebuffer->getSpecification().height == height)
        {
            return;
        }

        FramebufferSpecification gtaoSpec;
        gtaoSpec.width = width;
        gtaoSpec.height = height;
        gtaoSpec.attachments = {
            FramebufferTextureFormat::RG16F};
        gtaoSpec.swapChainTarget = false;

        m_framebuffer = Framebuffer::create(gtaoSpec);
    }

    void GTAORenderer::addPass(RenderGraphLegacy& renderGraph,
                                const RenderContext& context,
                                const GBufferRenderer& gBuffer,
                                ResourceHandle gtaoOutput,
                                const Settings& settings)
    {
        LegacyRenderGraphPass pass;
        pass.Name = "GTAOPass";
        pass.Inputs = {};  // GBuffer is accessed via gBuffer reference
        pass.Outputs = {gtaoOutput};
        pass.Execute = [this, &context, &gBuffer, settings](CommandBuffer& commandBuffer)
        {
            auto gBufferFramebuffer = gBuffer.getFramebuffer();
            if (!gBufferFramebuffer || !m_pipeline || !m_quadVA || !m_framebuffer)
                return;

            const glm::mat4 projection = context.camera.camera.getProjection();
            const glm::mat4 inverseProjection = glm::inverse(projection);
            const glm::mat4 view = context.camera.view;

            int sliceCount = settings.sliceCount;
            if (sliceCount < 1)
                sliceCount = 1;
            if (sliceCount > 12)
                sliceCount = 12;

            int stepCount = settings.stepCount;
            if (stepCount < 1)
                stepCount = 1;
            if (stepCount > 16)
                stepCount = 16;

            const float radius = settings.radius;
            const float bias = settings.bias;
            const float power = settings.power;
            const float intensity = settings.intensity;

            commandBuffer.record([this, gBufferFramebuffer, projection, inverseProjection, view,
                                  sliceCount, stepCount, radius, bias, power, intensity](RendererAPI& api)
            {
                if (!gBufferFramebuffer || !m_pipeline || !m_framebuffer || !m_quadVA)
                    return;

                m_framebuffer->bind();
                RenderCommand::setBlendEnabled(false);
                RenderCommand::setClearColor({1.0f, 1.0f, 1.0f, 1.0f});
                RenderCommand::clear();

                m_pipeline->bind();
                auto shader = m_pipeline->getShader();

                shader->setInt("u_GBufferNormal", 0);
                shader->setInt("u_GBufferDepth", 1);

                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal), 0);
                gBufferFramebuffer->bindDepthAttachment(1);

                shader->setMat4("u_Projection", projection);
                shader->setMat4("u_InverseProjection", inverseProjection);
                shader->setMat4("u_View", view);
                shader->setFloat("u_GTAORadius", radius);
                shader->setFloat("u_GTAOBias", bias);
                shader->setFloat("u_GTAOPower", power);
                shader->setFloat("u_GTAOIntensity", intensity);
                shader->setInt("u_GTAOSliceCount", sliceCount);
                shader->setInt("u_GTAOStepCount", stepCount);

                RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
                RenderCommand::setBlendEnabled(true);
            });
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
