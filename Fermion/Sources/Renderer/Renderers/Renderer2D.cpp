#include "fmpch.hpp"
#include "Renderer2D.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/UniformBuffer.hpp"
#include "Renderer/UniformBufferLayout.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/Texture/SubTexture2D.hpp"
#include "Renderer/Camera/OrthographicCamera.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/Font/Font.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include "Renderer/RenderCommandQueue.hpp"
#include "Renderer/RenderCommands.hpp"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Renderer.hpp"
#include "Renderer/Font/MSDFData.hpp"
#include <filesystem>

namespace Fermion
{
    Renderer2D::Renderer2D()
    {
    }

    Renderer2D::~Renderer2D()
    {
    }

    void Renderer2D::init(const RendererConfig& config)
    {
        FM_PROFILE_FUNCTION();

        // Create batch renderers
        m_QuadBatch = std::make_unique<QuadBatch>();
        m_CircleBatch = std::make_unique<CircleBatch>();
        m_LineBatch = std::make_unique<LineBatch>();
        m_TextBatch = std::make_unique<TextBatch>();

        // Initialize batch renderers
        m_QuadBatch->init();
        m_CircleBatch->init(m_QuadBatch->getIndexBuffer()); // Share index buffer
        m_LineBatch->init();
        m_TextBatch->init(m_QuadBatch->getIndexBuffer()); // Share index buffer

        // Create render graph and command queue
        m_RenderGraph = std::make_unique<RenderGraphLegacy>();
        m_CommandQueue = std::make_unique<RenderCommandQueue>();

        // Create pipelines
        // Quad Pipeline
        {
            PipelineSpecification spec;
            spec.shader = nullptr;
            spec.depthTest = true;
            spec.depthWrite = true;
            spec.cull = CullMode::None;
            spec.depthOperator = DepthCompareOperator::Less;
            m_QuadPipeline = Pipeline::create(spec);
        }

        // QuadInstance Pipeline
        {
            PipelineSpecification spec;
            spec.shader = nullptr;
            spec.depthTest = true;
            spec.depthWrite = true;
            spec.cull = CullMode::None;
            spec.depthOperator = DepthCompareOperator::Less;
            m_QuadInstancePipeline = Pipeline::create(spec);
        }

        // Circle Pipeline
        {
            PipelineSpecification spec;
            spec.shader = nullptr;
            spec.depthTest = true;
            spec.depthWrite = true;
            spec.cull = CullMode::None;
            spec.depthOperator = DepthCompareOperator::Less;
            m_CirclePipeline = Pipeline::create(spec);
        }

        // Line Pipeline
        {
            PipelineSpecification spec;
            spec.shader = nullptr;
            spec.depthTest = true;
            spec.depthWrite = true;
            spec.cull = CullMode::None;
            spec.depthOperator = DepthCompareOperator::Always;
            m_LinePipeline = Pipeline::create(spec);
        }

        // Text Pipeline
        {
            PipelineSpecification spec;
            spec.shader = nullptr;
            spec.depthTest = true;
            spec.depthWrite = true;
            spec.cull = CullMode::None;
            spec.depthOperator = DepthCompareOperator::Always;
            m_TextPipeline = Pipeline::create(spec);
        }

        // Load shaders
        m_QuadShader = Renderer::getShaderLibrary()->get("Quad");
        m_QuadInstanceShader = Renderer::getShaderLibrary()->get("QuadInstance");
        m_CircleShader = Renderer::getShaderLibrary()->get("Circle");
        m_LineShader = Renderer::getShaderLibrary()->get("Line");
        m_TextShader = Renderer::getShaderLibrary()->get("Text");

        // Set up texture samplers
        m_QuadShader->bind();
        int samplers[QuadBatch::MaxTextureSlots];
        for (uint32_t i = 0; i < QuadBatch::MaxTextureSlots; i++)
            samplers[i] = i;
        m_QuadShader->setIntArray("u_Textures", samplers, QuadBatch::MaxTextureSlots);

        m_QuadInstanceShader->bind();
        m_QuadInstanceShader->setIntArray("u_Textures", samplers, QuadBatch::MaxTextureSlots);

        m_TextShader->bind();
        m_TextShader->setInt("u_Atlas", 0);

        // Create camera uniform buffer (binding point 0)
        m_CameraUBO = UniformBuffer::create(UniformBufferBinding::Camera, CameraData::getSize());
    }

    void Renderer2D::shutdown()
    {
        FM_PROFILE_FUNCTION();

        if (m_QuadBatch) m_QuadBatch->shutdown();
        if (m_CircleBatch) m_CircleBatch->shutdown();
        if (m_LineBatch) m_LineBatch->shutdown();
        if (m_TextBatch) m_TextBatch->shutdown();
    }

    void Renderer2D::updateCameraUBO(const glm::mat4& viewProj, const glm::mat4& view,
                                     const glm::vec3& cameraPos)
    {
        CameraData cameraData;
        cameraData.viewProjection = viewProj;
        cameraData.view = view;
        cameraData.projection = glm::mat4(1.0f);
        cameraData.position = cameraPos;

        m_CameraUBO->setData(&cameraData, sizeof(CameraData));
    }

    void Renderer2D::resetBuffers()
    {
        m_QuadBatch->reset();
        m_CircleBatch->reset();
        m_LineBatch->reset();
        m_TextBatch->reset();
    }

    void Renderer2D::beginScene(const OrthographicCamera& camera)
    {
        FM_PROFILE_FUNCTION();
        updateCameraUBO(camera.getViewProjectionMatrix(), glm::mat4(1.0f), glm::vec3(0.0f));
        resetBuffers();
    }

    void Renderer2D::beginScene(const EditorCamera& camera)
    {
        FM_PROFILE_FUNCTION();
        m_CameraView = camera.getViewMatrix();
        m_CameraViewProj = camera.getViewProjection();
        updateCameraUBO(m_CameraViewProj, m_CameraView, camera.getPosition());
        resetBuffers();
    }

    void Renderer2D::beginScene(const Camera& camera, const glm::mat4& view)
    {
        FM_PROFILE_FUNCTION();
        m_CameraView = view;
        m_CameraViewProj = camera.getProjection() * view;
        glm::vec3 cameraPos = glm::vec3(glm::inverse(view)[3]);
        updateCameraUBO(m_CameraViewProj, m_CameraView, cameraPos);
        resetBuffers();
    }

    void Renderer2D::endScene()
    {
        FM_PROFILE_FUNCTION();
        flush();
    }

    void Renderer2D::flush()
    {
        FM_PROFILE_FUNCTION();

        m_RenderGraph->reset();

        if (m_QuadBatch->hasData())
            quadPass();

        if (m_QuadBatch->hasInstanceData())
            quadInstancePass();

        if (m_CircleBatch->hasData())
            circlePass();

        if (m_LineBatch->hasData())
            linePass();

        if (m_TextBatch->hasData())
            textPass();

        m_RenderGraph->execute(*m_CommandQueue, Renderer::getRendererAPI());
    }

    void Renderer2D::flushAndReset()
    {
        endScene();
        resetBuffers();
    }

    // --- Quad drawing ---

    void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        drawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        FM_PROFILE_FUNCTION();
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        drawQuad(transform, color);
    }

    void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture, float tilingFactor, glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture, float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        drawQuad(transform, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size,
                              const std::shared_ptr<SubTexture2D>& subtexture, float tilingFactor,
                              glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, subtexture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size,
                              const std::shared_ptr<SubTexture2D>& subtexture, float tilingFactor,
                              glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        const glm::vec2* txCoord = subtexture->getTexCoords();
        const std::shared_ptr<Texture2D>& texture = subtexture->getTexture();
        float textureIndex = m_QuadBatch->getTextureIndex(texture);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        m_QuadBatch->submit(transform, glm::vec4(1.0f), txCoord, textureIndex, tilingFactor, -1);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4& transform, const glm::vec4& color, int objectID)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        m_QuadBatch->submit(transform, color, QuadBatch::DefaultTexCoords, 0.0f, 1.0f, objectID);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture,
                              float tilingFactor, glm::vec4 tintColor, int objectID)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        float textureIndex = m_QuadBatch->getTextureIndex(texture);
        m_QuadBatch->submit(transform, tintColor, QuadBatch::DefaultTexCoords, textureIndex, tilingFactor, objectID);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4& transform, const std::shared_ptr<SubTexture2D>& subTexture,
                              float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        const glm::vec2* txCoord = subTexture->getTexCoords();
        const std::shared_ptr<Texture2D>& texture = subTexture->getTexture();
        float textureIndex = m_QuadBatch->getTextureIndex(texture);

        m_QuadBatch->submit(transform, glm::vec4(1.0f), txCoord, textureIndex, tilingFactor, -1);
        m_Stats.quadCount++;
    }

    // --- Billboard ---

    void Renderer2D::drawQuadBillboard(const glm::vec3& position, const glm::vec2& size,
                                       const glm::vec4& color, int objectId)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        glm::mat4 view = m_CameraView;
        view[3] = glm::vec4(0, 0, 0, 1);
        glm::mat4 billboardRotation = glm::inverse(view);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             billboardRotation *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        m_QuadBatch->submit(transform, color, QuadBatch::DefaultTexCoords, 0.0f, 1.0f, objectId);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawQuadBillboard(const glm::vec3& position, const glm::vec2& size,
                                       const std::shared_ptr<Texture2D>& texture,
                                       float tilingFactor, const glm::vec4& tintColor, int objectId)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        glm::mat4 view = m_CameraView;
        view[3] = glm::vec4(0, 0, 0, 1);
        glm::mat4 billboardRotation = glm::inverse(view);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             billboardRotation *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        float textureIndex = m_QuadBatch->getTextureIndex(texture);
        m_QuadBatch->submit(transform, tintColor, QuadBatch::DefaultTexCoords, textureIndex, tilingFactor, objectId);
        m_Stats.quadCount++;
    }

    // --- AABB ---

    void Renderer2D::drawAABB(const AABB& aabb, const glm::mat4& transform, const glm::vec4& color, int objectId)
    {
        FM_PROFILE_FUNCTION();

        glm::vec4 corners[8] = {
            transform * glm::vec4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
            transform * glm::vec4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
            transform * glm::vec4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
            transform * glm::vec4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},

            transform * glm::vec4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
            transform * glm::vec4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
            transform * glm::vec4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
            transform * glm::vec4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f}
        };

        for (uint32_t i = 0; i < 4; i++)
            drawLine(corners[i], corners[(i + 1) % 4], color, objectId);

        for (uint32_t i = 0; i < 4; i++)
            drawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color, objectId);

        for (uint32_t i = 0; i < 4; i++)
            drawLine(corners[i], corners[i + 4], color, objectId);
    }

    // --- Instanced quad ---

    void Renderer2D::drawQuadInstanced(const glm::mat4& transform, const glm::vec4& color,
                                       const std::shared_ptr<Texture2D>& texture, float tilingFactor, int objectID)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isInstanceFull())
            flushAndReset();

        float textureIndex = m_QuadBatch->getTextureIndex(texture);
        m_QuadBatch->submitInstanced(transform, color, textureIndex, tilingFactor, objectID);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawQuadInstanced(const glm::mat4& transform, const glm::vec4& color, int objectID)
    {
        drawQuadInstanced(transform, color, m_QuadBatch->getWhiteTexture(), 1.0f, objectID);
    }

    // --- Rotated quad ---

    void Renderer2D::drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                                     const glm::vec4& color)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, color);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                                     const glm::vec4& color)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        m_QuadBatch->submit(transform, color, QuadBatch::DefaultTexCoords, 0.0f, 1.0f, -1);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                                     const std::shared_ptr<Texture2D>& texture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                                     const std::shared_ptr<Texture2D>& texture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        float textureIndex = m_QuadBatch->getTextureIndex(texture);
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        m_QuadBatch->submit(transform, glm::vec4(1.0f), QuadBatch::DefaultTexCoords, textureIndex, tilingFactor, -1);
        m_Stats.quadCount++;
    }

    void Renderer2D::drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                                     const std::shared_ptr<SubTexture2D>& subtexture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, subtexture, tilingFactor, tintColor);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                                     const std::shared_ptr<SubTexture2D>& subtexture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        if (m_QuadBatch->isFull())
            flushAndReset();

        const std::shared_ptr<Texture2D>& texture = subtexture->getTexture();
        float textureIndex = m_QuadBatch->getTextureIndex(texture);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                             glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) *
                             glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        const glm::vec2* txCoord = subtexture->getTexCoords();
        m_QuadBatch->submit(transform, glm::vec4(1.0f), txCoord, textureIndex, tilingFactor, -1);
        m_Stats.quadCount++;
    }

    // --- Circle ---

    void Renderer2D::drawCircle(const glm::mat4& transform, const glm::vec4& color,
                                float thickness, float fade, int objectID)
    {
        FM_PROFILE_FUNCTION();
        m_CircleBatch->submit(transform, color, thickness, fade, objectID,
                             m_QuadBatch->getQuadVertexPositions());
        m_Stats.circleCount++;
    }

    // --- Line ---

    void Renderer2D::drawLine(const glm::vec3& p0, const glm::vec3& p1,
                              const glm::vec4& color, int objectID)
    {
        m_LineBatch->submit(p0, p1, color, objectID);
        m_Stats.lineCount++;
    }

    void Renderer2D::drawRect(const glm::vec3& position, const glm::vec2& size,
                              const glm::vec4& color, int objectID)
    {
        glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
        glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

        drawLine(p0, p1, color, objectID);
        drawLine(p1, p2, color, objectID);
        drawLine(p2, p3, color, objectID);
        drawLine(p3, p0, color, objectID);
    }

    void Renderer2D::drawRect(const glm::mat4& transform, const glm::vec4& color, int objectID)
    {
        const glm::vec4* positions = m_QuadBatch->getQuadVertexPositions();
        glm::vec3 lineVertices[4];
        for (size_t i = 0; i < 4; i++)
            lineVertices[i] = transform * positions[i];

        setLineWidth(1.0f);
        drawLine(lineVertices[0], lineVertices[1], color, objectID);
        drawLine(lineVertices[1], lineVertices[2], color, objectID);
        drawLine(lineVertices[2], lineVertices[3], color, objectID);
        drawLine(lineVertices[3], lineVertices[0], color, objectID);
    }

    float Renderer2D::getLineWidth()
    {
        return m_LineBatch->getLineWidth();
    }

    void Renderer2D::setLineWidth(float width)
    {
        m_LineBatch->setLineWidth(width);
    }

    // --- Text ---

    void Renderer2D::drawString(const std::string& string, std::shared_ptr<Font> font,
                                const glm::mat4& transform, const TextParams& textParams, int objectId)
    {
        m_TextBatch->submit(string, font, transform, textParams, objectId);
        // Note: TextBatch internally tracks quad count via index count
        // We approximate stats here as the original did
        // (TextBatch adds 6 indices per glyph = 1 quad)
    }

    // --- Outline pass ---

    void Renderer2D::recordOutlinePass(RenderCommandQueue& queue,
                                       const std::vector<MeshDrawCommand>& drawCommands,
                                       const glm::vec4& outlineColor)
    {
        for (auto& cmd : drawCommands)
        {
            if (cmd.drawOutline && cmd.visible)
            {
                drawAABB(cmd.aabb, cmd.transform, outlineColor, cmd.objectID);
            }
        }
    }

    // --- Statistics ---

    void Renderer2D::resetStatistics()
    {
        memset(&m_Stats, 0, sizeof(Satistics));
    }

    Renderer2D::Satistics Renderer2D::getStatistics() const
    {
        return m_Stats;
    }

    // --- Render passes ---

    void Renderer2D::quadPass()
    {
        // Capture 'this' pointer for lambda
        auto* self = this;

        LegacyRenderGraphPass pass;
        pass.Name = "QuadPass";
        pass.Execute = [self](RenderCommandQueue& queue) {
            queue.submit(CmdCustom{[self]() {
                self->m_QuadBatch->uploadToGPU();
                self->m_QuadBatch->bindTextures();
                self->m_QuadShader->bind();
            }});
            queue.submit(CmdBindPipeline{self->m_QuadPipeline});
            queue.submit(CmdDrawIndexed{self->m_QuadBatch->getVertexArray(),
                                        self->m_QuadBatch->getIndexCount()});
            self->m_Stats.drawCalls++;
        };
        m_RenderGraph->addPass(pass);
    }

    void Renderer2D::quadInstancePass()
    {
        auto* self = this;

        LegacyRenderGraphPass pass;
        pass.Name = "QuadInstancePass";
        pass.Execute = [self](RenderCommandQueue& queue) {
            queue.submit(CmdCustom{[self]() {
                self->m_QuadBatch->uploadInstanceDataToGPU();
                self->m_QuadBatch->bindTextures();
                self->m_QuadInstanceShader->bind();
            }});
            queue.submit(CmdBindPipeline{self->m_QuadInstancePipeline});
            queue.submit(CmdDrawIndexedInstanced{self->m_QuadBatch->getInstanceVertexArray(),
                                                 6, self->m_QuadBatch->getInstanceCount()});
            self->m_Stats.drawCalls++;
        };
        m_RenderGraph->addPass(pass);
    }

    void Renderer2D::circlePass()
    {
        auto* self = this;

        LegacyRenderGraphPass pass;
        pass.Name = "CirclePass";
        pass.Execute = [self](RenderCommandQueue& queue) {
            queue.submit(CmdCustom{[self]() {
                self->m_CircleBatch->uploadToGPU();
                self->m_CircleShader->bind();
            }});
            queue.submit(CmdBindPipeline{self->m_CirclePipeline});
            queue.submit(CmdDrawIndexed{self->m_CircleBatch->getVertexArray(),
                                        self->m_CircleBatch->getIndexCount()});
            self->m_Stats.drawCalls++;
        };
        m_RenderGraph->addPass(pass);
    }

    void Renderer2D::linePass()
    {
        auto* self = this;

        LegacyRenderGraphPass pass;
        pass.Name = "LinePass";
        pass.Execute = [self](RenderCommandQueue& queue) {
            queue.submit(CmdCustom{[self]() {
                self->m_LineBatch->uploadToGPU();
                self->m_LineShader->bind();
            }});
            queue.submit(CmdBindPipeline{self->m_LinePipeline});
            queue.submit(CmdSetLineWidth{self->m_LineBatch->getLineWidth()});
            queue.submit(CmdDrawLines{self->m_LineBatch->getVertexArray(),
                                      self->m_LineBatch->getVertexCount()});
            self->m_Stats.drawCalls++;
        };
        m_RenderGraph->addPass(pass);
    }

    void Renderer2D::textPass()
    {
        auto* self = this;

        LegacyRenderGraphPass pass;
        pass.Name = "TextPass";
        pass.Execute = [self](RenderCommandQueue& queue) {
            queue.submit(CmdCustom{[self]() {
                self->m_TextBatch->uploadToGPU();
                self->m_TextBatch->bindFontAtlas();
                self->m_TextShader->bind();
            }});
            queue.submit(CmdBindPipeline{self->m_TextPipeline});
            queue.submit(CmdDrawIndexed{self->m_TextBatch->getVertexArray(),
                                        self->m_TextBatch->getIndexCount()});
            self->m_Stats.drawCalls++;
        };
        m_RenderGraph->addPass(pass);
    }

} // namespace Fermion
