#include "SSGIRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommands.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/VertexArray.hpp"

namespace Fermion
{
    namespace
    {
        bool HasMatrixChanged(const glm::mat4& a, const glm::mat4& b, float epsilon)
        {
            for (int col = 0; col < 4; ++col)
            {
                for (int row = 0; row < 4; ++row)
                {
                    if (std::abs(a[col][row] - b[col][row]) > epsilon)
                        return true;
                }
            }
            return false;
        }
    }

    SSGIRenderer::SSGIRenderer()
    {
        // SSGI Pipeline
        {
            PipelineSpecification ssgiSpec;
            ssgiSpec.shader = Renderer::getShaderLibrary()->get("SSGI");
            ssgiSpec.depthTest = false;
            ssgiSpec.depthWrite = false;
            ssgiSpec.cull = CullMode::None;

            m_pipeline = Pipeline::create(ssgiSpec);
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

    void SSGIRenderer::ensureFramebuffers(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        const bool hasFramebuffers = m_framebuffers[0] && m_framebuffers[1];
        if (hasFramebuffers &&
            m_framebuffers[0]->getSpecification().width == width &&
            m_framebuffers[0]->getSpecification().height == height)
            return;

        FramebufferSpecification ssgiSpec;
        ssgiSpec.width = width;
        ssgiSpec.height = height;
        ssgiSpec.attachments = {
            FramebufferTextureFormat::RGB16F};
        ssgiSpec.swapChainTarget = false;

        m_framebuffers[0] = Framebuffer::create(ssgiSpec);
        m_framebuffers[1] = Framebuffer::create(ssgiSpec);
        m_currentFramebuffer = m_framebuffers[0];
        m_historyIndex = 0;
        m_frameIndex = 0;
        m_historyValid = false;
        m_wasEnabled = false;
    }

    void SSGIRenderer::resetAccumulation()
    {
        m_frameIndex = 0;
        m_historyValid = false;
    }

    bool SSGIRenderer::hasSettingsChanged(const Settings& settings, const glm::mat4& viewProjection) const
    {
        const float epsilon = 1e-4f;

        if (HasMatrixChanged(viewProjection, m_lastViewProjection, epsilon))
            return true;

        if (std::abs(settings.radius - m_lastSettings.radius) > epsilon ||
            std::abs(settings.bias - m_lastSettings.bias) > epsilon ||
            std::abs(settings.intensity - m_lastSettings.intensity) > epsilon ||
            settings.sampleCount != m_lastSettings.sampleCount)
            return true;

        return false;
    }

    void SSGIRenderer::addPass(RenderGraphLegacy& renderGraph,
                                const RenderContext& context,
                                const GBufferRenderer& gBuffer,
                                ResourceHandle ssgiOutput,
                                const Settings& settings)
    {
        LegacyRenderGraphPass pass;
        pass.Name = "SSGIPass";
        pass.Inputs = {};  // GBuffer is accessed via gBuffer reference
        pass.Outputs = {ssgiOutput};
        pass.Execute = [this, &context, &gBuffer, settings](RenderCommandQueue& queue)
        {
            auto gBufferFramebuffer = gBuffer.getFramebuffer();
            if (!gBufferFramebuffer || !m_pipeline || !m_quadVA || !m_framebuffers[0] || !m_framebuffers[1])
                return;

            const glm::mat4 viewProjection = context.camera.camera.getProjection() * context.camera.view;
            const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

            bool resetAccum = !m_historyValid || !m_wasEnabled;
            if (!resetAccum && hasSettingsChanged(settings, viewProjection))
                resetAccum = true;

            if (resetAccum)
                m_frameIndex = 0;

            int sampleCount = settings.sampleCount;
            if (sampleCount < 1)
                sampleCount = 1;
            if (sampleCount > 64)
                sampleCount = 64;

            const uint32_t historyIndex = m_historyIndex;
            const uint32_t currentIndex = (historyIndex + 1) % 2;
            auto historyFramebuffer = m_framebuffers[historyIndex];
            auto currentFramebuffer = m_framebuffers[currentIndex];
            if (!historyFramebuffer || !currentFramebuffer)
                return;

            const int frameIndex = static_cast<int>(m_frameIndex);
            const float radius = settings.radius;
            const float bias = settings.bias;
            const float intensity = settings.intensity;

            queue.submit(CmdBindFramebuffer{currentFramebuffer});
            queue.submit(CmdSetBlendEnabled{false});
            queue.submit(CmdSetClearColor{{0.0f, 0.0f, 0.0f, 1.0f}});
            queue.submit(CmdClear{});

            queue.submit(CmdCustom{[this, gBufferFramebuffer, historyFramebuffer,
                                    viewProjection, inverseViewProjection,
                                    sampleCount, radius, bias, intensity, frameIndex]() {
                m_pipeline->bind();
                auto shader = m_pipeline->getShader();

                shader->setInt("u_GBufferAlbedo", 0);
                shader->setInt("u_GBufferNormal", 1);
                shader->setInt("u_GBufferDepth", 2);
                shader->setInt("u_SSGIHistory", 3);

                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Albedo), 0);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal), 1);
                gBufferFramebuffer->bindDepthAttachment(2);
                historyFramebuffer->bindColorAttachment(0, 3);

                shader->setMat4("u_ViewProjection", viewProjection);
                shader->setMat4("u_InverseViewProjection", inverseViewProjection);

                shader->setInt("u_SSGISampleCount", sampleCount);
                shader->setFloat("u_SSGIRadius", radius);
                shader->setFloat("u_SSGIBias", bias);
                shader->setFloat("u_SSGIIntensity", intensity);
                shader->setInt("u_FrameIndex", frameIndex);
            }});

            queue.submit(CmdDrawIndexed{m_quadVA, m_quadVA->getIndexBuffer()->getCount()});
            queue.submit(CmdUnbindFramebuffer{currentFramebuffer});
            queue.submit(CmdSetBlendEnabled{true});

            m_currentFramebuffer = currentFramebuffer;
            m_historyIndex = currentIndex;
            m_historyValid = true;
            m_wasEnabled = true;
            m_lastViewProjection = viewProjection;
            m_lastSettings = settings;

            m_frameIndex++;
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
