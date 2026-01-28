#include "OutlineRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer2D.hpp"

namespace Fermion
{
    OutlineRenderer::OutlineRenderer()
    {
        // G-Buffer Outline Pipeline
        {
            PipelineSpecification outlineSpec;
            outlineSpec.shader = Renderer::getShaderLibrary()->get("GBufferOutline");
            outlineSpec.depthTest = false;
            outlineSpec.depthWrite = false;
            outlineSpec.cull = CullMode::None;

            m_pipeline = Pipeline::create(outlineSpec);
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

    void OutlineRenderer::addPass(RenderGraphLegacy& renderGraph,
                                   const RenderContext& context,
                                   const GBufferRenderer* gBuffer,
                                   const std::vector<MeshDrawCommand>& drawList,
                                   const std::vector<int>& outlineIDs,
                                   const Settings& settings,
                                   ResourceHandle gBufferHandle,
                                   ResourceHandle sceneDepth,
                                   ResourceHandle lightingResult)
    {
        // Collect all outline IDs from both explicit list and draw commands
        std::vector<int> allOutlineIDs = outlineIDs;
        bool hasOutlineDrawCommands = false;
        for (const auto& cmd : drawList)
        {
            if (cmd.drawOutline && cmd.visible)
            {
                hasOutlineDrawCommands = true;
                allOutlineIDs.push_back(cmd.objectID);
            }
        }

        // Deduplicate and limit outline IDs
        std::vector<int> uniqueIDs;
        uniqueIDs.reserve(allOutlineIDs.size());
        const size_t maxOutlineIDs = 32;
        for (int id : allOutlineIDs)
        {
            if (id < 0)
                continue;

            bool exists = false;
            for (int existing : uniqueIDs)
            {
                if (existing == id)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                uniqueIDs.push_back(id);

            if (uniqueIDs.size() >= maxOutlineIDs)
                break;
        }

        const bool canUseGBuffer = gBuffer && gBuffer->getFramebuffer() && m_pipeline;

        if (canUseGBuffer && !uniqueIDs.empty())
        {
            LegacyRenderGraphPass pass;
            pass.Name = "OutlinePass";
            pass.Inputs = {gBufferHandle, sceneDepth, lightingResult};
            pass.Execute = [this, &context, gBuffer, uniqueIDs = std::move(uniqueIDs), settings](CommandBuffer& commandBuffer)
            {
                commandBuffer.record([this, &context, gBuffer, uniqueIDs, settings](RendererAPI& api)
                {
                    auto gBufferFramebuffer = gBuffer->getFramebuffer();
                    if (!gBufferFramebuffer || !m_pipeline || uniqueIDs.empty())
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

                    m_pipeline->bind();
                    auto shader = m_pipeline->getShader();

                    shader->setInt("u_GBufferNormal", 0);
                    shader->setInt("u_GBufferDepth", 1);
                    shader->setInt("u_GBufferObjectID", 2);

                    gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal), 0);
                    gBufferFramebuffer->bindDepthAttachment(1);
                    gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::ObjectID), 2);

                    shader->setFloat4("u_OutlineColor", settings.color);
                    int outlineCount = static_cast<int>(uniqueIDs.size());
                    if (outlineCount > 32)
                        outlineCount = 32;
                    shader->setInt("u_OutlineIDCount", outlineCount);
                    if (outlineCount > 0)
                        shader->setIntArray("u_OutlineIDs", const_cast<int*>(uniqueIDs.data()), outlineCount);

                    shader->setFloat("u_Near", context.camera.nearClip);
                    shader->setFloat("u_Far", context.camera.farClip);
                    shader->setFloat("u_DepthThreshold", settings.depthThreshold);
                    shader->setFloat("u_NormalThreshold", settings.normalThreshold);
                    shader->setFloat("u_Thickness", settings.thickness);

                    RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
                });
            };
            renderGraph.addPass(pass);
            return;
        }

        // Fallback to 2D outline rendering
        if (!hasOutlineDrawCommands)
            return;

        LegacyRenderGraphPass pass;
        pass.Name = "OutlinePass";
        pass.Inputs = {lightingResult};
        pass.Execute = [&drawList, settings](CommandBuffer& commandBuffer)
        {
            Renderer2D::recordOutlinePass(commandBuffer, drawList, settings.color);
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
