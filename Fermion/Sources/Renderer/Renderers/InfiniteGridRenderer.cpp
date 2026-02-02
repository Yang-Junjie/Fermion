#include "InfiniteGridRenderer.hpp"
#include "RenderContext.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/RenderDrawCommand.hpp"

namespace Fermion
{
    InfiniteGridRenderer::InfiniteGridRenderer()
    {
        initializeResources();
    }

    void InfiniteGridRenderer::initializeResources()
    {
        if (m_initialized)
            return;

        // Create fullscreen quad for grid rendering
        float quadVertices[] = {
            // positions (NDC)
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f
        };

        uint32_t quadIndices[] = {
            0, 1, 2,
            2, 3, 0
        };

        m_quadVA = VertexArray::create();

        std::shared_ptr<VertexBuffer> quadVB = VertexBuffer::create(quadVertices, sizeof(quadVertices));
        quadVB->setLayout({
            {ShaderDataType::Float3, "a_Position"}
        });
        m_quadVA->addVertexBuffer(quadVB);

        std::shared_ptr<IndexBuffer> quadIB = IndexBuffer::create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
        m_quadVA->setIndexBuffer(quadIB);

        // Create pipeline
        PipelineSpecification pipelineSpec;
        pipelineSpec.shader = Shader::create("../Boson/Resources/Shaders/InfiniteGrid.glsl");
        pipelineSpec.depthTest = true;
        pipelineSpec.depthWrite = false;  // Transparent grid should not write to depth buffer
        pipelineSpec.cull = CullMode::None;

        m_gridPipeline = Pipeline::create(pipelineSpec);

        m_initialized = true;
    }

    void InfiniteGridRenderer::addPass(RenderGraphLegacy& renderGraph,
                                       const RenderContext& context,
                                       const Settings& settings,
                                       ResourceHandle colorTarget,
                                       ResourceHandle depthTarget)
    {
        if (!settings.enabled || !m_initialized)
            return;

        LegacyRenderGraphPass pass;
        pass.Name = "InfiniteGrid";
        // Only read from dependencies, don't produce new outputs
        if (colorTarget.isValid())
            pass.Inputs = {colorTarget};
        if (depthTarget.isValid())
            pass.Inputs.push_back(depthTarget);

        pass.Execute = [this, context, settings](CommandBuffer& commandBuffer) {
            commandBuffer.record([this, context, settings](RendererAPI&) {
                render(context, settings);
            });
        };

        renderGraph.addPass(pass);
    }

    void InfiniteGridRenderer::render(const RenderContext& context, const Settings& settings)
    {
        if (!m_initialized || !m_gridPipeline || !m_quadVA)
            return;

        // Enable alpha blending for anti-aliased grid lines
        RenderCommand::setBlendEnabled(true);

        m_gridPipeline->bind();

        auto shader = m_gridPipeline->getShader();
        shader->setInt("u_GridPlane", static_cast<int>(settings.plane));
        shader->setFloat("u_GridScale", settings.gridScale);
        shader->setFloat("u_FadeDistance", settings.fadeDistance);
        shader->setFloat4("u_GridColorThin", settings.gridColorThin);
        shader->setFloat4("u_GridColorThick", settings.gridColorThick);
        shader->setFloat4("u_AxisColorX", settings.axisColorX);
        shader->setFloat4("u_AxisColorZ", settings.axisColorZ);

        RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
    }

} // namespace Fermion
