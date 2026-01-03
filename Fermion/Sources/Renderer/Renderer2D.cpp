#include "fmpch.hpp"
#include "Renderer2D.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"
#include "RenderCommand.hpp"
#include "Renderer/Pipeline.hpp"
#include "glad/glad.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Renderer2D.hpp"
#include "Font/MSDFData.hpp"
#include <filesystem>

namespace Fermion
{
    struct QuadInstanceData
    {
        glm::mat4 transform;
        glm::vec4 color;
        float texIndex;
        float tilingFactor;

        int objectID;
    };

    struct QuadVertices
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 txCoord;
        float texIndex;
        float tilingFactor;

        // editor only
        int objectID;
    };

    struct CircleVertices
    {
        glm::vec3 worldPosition;
        glm::vec3 localPosition;
        glm::vec4 color;
        float thickness;
        float fade;

        // Editor-only
        int objectID;
    };

    struct LineVertex
    {
        glm::vec3 position;
        glm::vec4 color;

        // Editor-only
        int objectID;
    };

    struct TextVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;

        // TODO: bg color for outline/bg

        // Editor-only
        int objectID;
    };

    struct Renderer2DData
    {
        std::shared_ptr<Pipeline> OutlinePipeline;
        std::shared_ptr<Pipeline> DefaultPipeline;

        static const uint32_t MaxQuads = 10000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32;

        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<VertexBuffer> QuadVertexBuffer;
        std::shared_ptr<Shader> QuadShader;
        std::shared_ptr<Texture2D> WhiteTexture;

        std::shared_ptr<VertexArray> QuadInstanceVertexArray;
        std::shared_ptr<VertexBuffer> QuadInstanceVertexBuffer;
        std::shared_ptr<Shader> QuadInstanceShader;

        std::shared_ptr<VertexArray> CircleVertexArray;
        std::shared_ptr<VertexBuffer> CircleVertexBuffer;
        std::shared_ptr<Shader> CircleShader;

        std::shared_ptr<VertexArray> LineVertexArray;
        std::shared_ptr<VertexBuffer> LineVertexBuffer;
        std::shared_ptr<Shader> LineShader;

        std::shared_ptr<VertexArray> TextVertexArray;
        std::shared_ptr<VertexBuffer> TextVertexBuffer;
        std::shared_ptr<Shader> TextShader;

        uint32_t QuadIndexCount = 0;
        QuadVertices *QuadVertexBufferBase = nullptr;
        QuadVertices *QuadVertexBufferPtr = nullptr;

        uint32_t QuadInstanceCount = 0;
        QuadInstanceData *QuadInstanceVertexBufferBase = nullptr;
        QuadInstanceData *QuadInstanceVertexBufferPtr = nullptr;

        uint32_t CircleIndexCount = 0;
        CircleVertices *CircleVertexBufferBase = nullptr;
        CircleVertices *CircleVertexBufferPtr = nullptr;

        uint32_t LineVertexCount = 0;
        LineVertex *LineVertexBufferBase = nullptr;
        LineVertex *LineVertexBufferPtr = nullptr;

        uint32_t TextIndexCount = 0;
        TextVertex *TextVertexBufferBase = nullptr;
        TextVertex *TextVertexBufferPtr = nullptr;

        float LineWidth = 2.0f;

        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        std::shared_ptr<Texture2D> FontAtlasTexture;

        glm::vec4 QuadVertexPositions[4];

        Renderer2D::Satistics stats;

        glm::mat4 m_CameraViewProj;
        glm::mat4 m_CameraView;
    };

    static Renderer2DData s_Data;

    void Renderer2D::init(const RendererConfig &config)
    {
        FM_PROFILE_FUNCTION();

        // Outline Pipeline
        {
            PipelineSpecification outlineSpec;
            // outlineSpec.Shader = Shader::create(config.ShaderPath + "Outline.glsl");
            outlineSpec.DepthTest = false;
            outlineSpec.DepthWrite = false; 
            outlineSpec.Cull = CullMode::None; 
            outlineSpec.DepthOperator = DepthCompareOperator::Always; 

            s_Data.OutlinePipeline = Pipeline::Create(outlineSpec);
        }

        // default pipeline
        {
            PipelineSpecification defaultSpec;
            defaultSpec.Cull = CullMode::None;
            s_Data.DefaultPipeline = Pipeline::Create(defaultSpec);
        }


        s_Data.QuadVertexArray = VertexArray::create();
        s_Data.QuadVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(QuadVertices));
        s_Data.QuadVertexBuffer->setLayout({{ShaderDataType::Float3, "a_Position"},
                                            {ShaderDataType::Float4, "a_Color"},
                                            {ShaderDataType::Float2, "a_TexCoord"},
                                            {ShaderDataType::Float, "a_TexIndex"},
                                            {ShaderDataType::Float, "a_TilingFactor"},
                                            {ShaderDataType::Int, "a_ObjectID"}});
        s_Data.QuadVertexArray->addVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = new QuadVertices[s_Data.MaxVertices];

        uint32_t *quadIndeices = new uint32_t[s_Data.MaxIndices];
        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6)
        {
            quadIndeices[i + 0] = offset + 0;
            quadIndeices[i + 1] = offset + 1;
            quadIndeices[i + 2] = offset + 2;

            quadIndeices[i + 3] = offset + 2;
            quadIndeices[i + 4] = offset + 3;
            quadIndeices[i + 5] = offset + 0;
            offset += 4;
        }

        std::shared_ptr<IndexBuffer> quadIB = IndexBuffer::create(quadIndeices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->setIndexBuffer(quadIB);
        delete[] quadIndeices;

        s_Data.QuadInstanceVertexArray = VertexArray::create();
        s_Data.QuadInstanceVertexBuffer = VertexBuffer::create(Renderer2DData::MaxQuads * sizeof(QuadInstanceData));
        s_Data.QuadInstanceVertexBuffer->setLayout({{ShaderDataType::Mat4, "a_Transform", false, 1},
                                                    {ShaderDataType::Float4, "a_Color", false, 1},
                                                    {ShaderDataType::Float, "a_TexIndex", false, 1},
                                                    {ShaderDataType::Float, "a_TilingFactor", false, 1},
                                                    {ShaderDataType::Int, "a_ObjectID", false, 1}});
        s_Data.QuadInstanceVertexArray->addVertexBuffer(s_Data.QuadInstanceVertexBuffer);
        s_Data.QuadInstanceVertexArray->setIndexBuffer(quadIB);
        s_Data.QuadInstanceVertexBufferBase = new QuadInstanceData[Renderer2DData::MaxQuads];
        s_Data.QuadInstanceVertexBufferPtr = s_Data.QuadInstanceVertexBufferBase;

        s_Data.CircleVertexArray = VertexArray::create();
        s_Data.CircleVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(CircleVertices));
        s_Data.CircleVertexBuffer->setLayout({{ShaderDataType::Float3, "a_WorldPosition"},
                                              {ShaderDataType::Float3, "a_LocalPosition"},
                                              {ShaderDataType::Float4, "a_Color"},
                                              {ShaderDataType::Float, "a_Thickness"},
                                              {ShaderDataType::Float, "a_Fade"},
                                              {ShaderDataType::Int, "a_ObjectID"}});
        s_Data.CircleVertexArray->addVertexBuffer(s_Data.CircleVertexBuffer);
        s_Data.CircleVertexArray->setIndexBuffer(quadIB);
        s_Data.CircleVertexBufferBase = new CircleVertices[s_Data.MaxVertices];
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        // Lines
        s_Data.LineVertexArray = VertexArray::create();
        s_Data.LineVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(LineVertex));
        s_Data.LineVertexBuffer->setLayout({{ShaderDataType::Float3, "a_Position"},
                                            {ShaderDataType::Float4, "a_Color"},
                                            {ShaderDataType::Int, "a_ObjectID"}});
        s_Data.LineVertexArray->addVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase = new LineVertex[s_Data.MaxVertices];

        // Text
        s_Data.TextVertexArray = VertexArray::create();
        s_Data.TextVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(TextVertex));
        s_Data.TextVertexBuffer->setLayout({{ShaderDataType::Float3, "a_Position"},
                                            {ShaderDataType::Float4, "a_Color"},
                                            {ShaderDataType::Float2, "a_TexCoord"},
                                            {ShaderDataType::Int, "a_ObjectID"}});
        s_Data.TextVertexArray->addVertexBuffer(s_Data.TextVertexBuffer);
        s_Data.TextVertexArray->setIndexBuffer(quadIB);
        s_Data.TextVertexBufferBase = new TextVertex[s_Data.MaxVertices];

        s_Data.WhiteTexture = Texture2D::create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->setData(&whiteTextureData, sizeof(uint32_t));

        std::filesystem::path shaderBase = config.ShaderPath;
        s_Data.QuadShader = Shader::create((shaderBase / "Quad.glsl").string());
        s_Data.QuadInstanceShader = Shader::create((shaderBase / "QuadInstance.glsl").string());
        s_Data.CircleShader = Shader::create((shaderBase / "Circle.glsl").string());
        s_Data.LineShader = Shader::create((shaderBase / "Line.glsl").string());
        s_Data.TextShader = Shader::create((shaderBase / "Text.glsl").string());

        s_Data.QuadShader->bind();
        int samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;
        s_Data.QuadShader->setIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        s_Data.QuadInstanceShader->bind();
        s_Data.QuadInstanceShader->setIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        s_Data.TextShader->bind();
        s_Data.TextShader->setInt("u_Atlas", 0);

        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    }

    void Renderer2D::shutdown()
    {
        FM_PROFILE_FUNCTION();
        delete[] s_Data.QuadVertexBufferBase;
        delete[] s_Data.QuadInstanceVertexBufferBase;
        delete[] s_Data.CircleVertexBufferBase;
        delete[] s_Data.LineVertexBufferBase;
        delete[] s_Data.TextVertexBufferBase;
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.getViewProjectionMatrix();

        s_Data.QuadShader->bind();
        s_Data.QuadShader->setMat4("u_ViewProjection", viewProj);

        s_Data.QuadInstanceShader->bind();
        s_Data.QuadInstanceShader->setMat4("u_ViewProjection", viewProj);

        s_Data.CircleShader->bind();
        s_Data.CircleShader->setMat4("u_ViewProjection", viewProj);

        s_Data.LineShader->bind();
        s_Data.LineShader->setMat4("u_ViewProjection", viewProj);

        s_Data.TextShader->bind();
        s_Data.TextShader->setMat4("u_ViewProjection", viewProj);

        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.QuadIndexCount = 0;
        s_Data.QuadInstanceVertexBufferPtr = s_Data.QuadInstanceVertexBufferBase;
        s_Data.QuadInstanceCount = 0;

        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
        s_Data.CircleIndexCount = 0;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextureSlotIndex = 1;

        s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;
        s_Data.TextIndexCount = 0;
    }

    void Renderer2D::beginScene(const EditorCamera &camera)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.getViewProjection();

        s_Data.QuadShader->bind();
        s_Data.QuadShader->setMat4("u_ViewProjection", viewProj);

        s_Data.m_CameraView = camera.getViewMatrix();
        s_Data.m_CameraViewProj = camera.getViewProjection();

        s_Data.QuadInstanceShader->bind();
        s_Data.QuadInstanceShader->setMat4("u_ViewProjection", viewProj);

        s_Data.CircleShader->bind();
        s_Data.CircleShader->setMat4("u_ViewProjection", viewProj);

        s_Data.LineShader->bind();
        s_Data.LineShader->setMat4("u_ViewProjection", viewProj);

        s_Data.TextShader->bind();
        s_Data.TextShader->setMat4("u_ViewProjection", viewProj);

        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.QuadIndexCount = 0;
        s_Data.QuadInstanceVertexBufferPtr = s_Data.QuadInstanceVertexBufferBase;
        s_Data.QuadInstanceCount = 0;

        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
        s_Data.CircleIndexCount = 0;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextureSlotIndex = 1;

        s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;
        s_Data.TextIndexCount = 0;
    }

    void Renderer2D::beginScene(const Camera &camera, const glm::mat4 &view)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.getProjection() * view;

        s_Data.QuadShader->bind();
        s_Data.QuadShader->setMat4("u_ViewProjection", viewProj);

        s_Data.m_CameraView = view;
        s_Data.m_CameraViewProj = viewProj;

        s_Data.QuadInstanceShader->bind();
        s_Data.QuadInstanceShader->setMat4("u_ViewProjection", viewProj);

        s_Data.CircleShader->bind();
        s_Data.CircleShader->setMat4("u_ViewProjection", viewProj);

        s_Data.LineShader->bind();
        s_Data.LineShader->setMat4("u_ViewProjection", viewProj);

        s_Data.TextShader->bind();
        s_Data.TextShader->setMat4("u_ViewProjection", viewProj);

        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.QuadIndexCount = 0;
        s_Data.QuadInstanceVertexBufferPtr = s_Data.QuadInstanceVertexBufferBase;
        s_Data.QuadInstanceCount = 0;

        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;
        s_Data.CircleIndexCount = 0;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextureSlotIndex = 1;

        s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;
        s_Data.TextIndexCount = 0;
    }

    void Renderer2D::endScene()
    {
        FM_PROFILE_FUNCTION();
        flush();
    }

    void Renderer2D::flush()
    {
        FM_PROFILE_FUNCTION();

        s_Data.DefaultPipeline->Bind();
        // glFrontFace(GL_CCW);
        // TODO:，当数量大的时候切换为实例化渲染
        if (s_Data.QuadIndexCount)
        {
            // Log::Info("Renderer2D: Batch rendering 1 batch");
            uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.QuadVertexBufferPtr - (uint8_t *)s_Data.QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->setData(s_Data.QuadVertexBufferBase, dataSize);

            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->bind(i);

            s_Data.QuadShader->bind();
            RenderCommand::drawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.stats.drawCalls++;
        }

        if (s_Data.QuadInstanceCount)
        {
            // Log::Info("Renderer2D: Instance rendering");
            uint32_t dataSize = s_Data.QuadInstanceCount * sizeof(QuadInstanceData);
            s_Data.QuadInstanceVertexBuffer->setData(s_Data.QuadInstanceVertexBufferBase, dataSize);

            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->bind(i);

            s_Data.QuadInstanceShader->bind();
            RenderCommand::drawIndexedInstanced(
                s_Data.QuadInstanceVertexArray, 6, s_Data.QuadInstanceCount);

            s_Data.stats.drawCalls++;
        }

        if (s_Data.CircleIndexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.CircleVertexBufferPtr - (uint8_t *)s_Data.CircleVertexBufferBase);
            s_Data.CircleVertexBuffer->setData(s_Data.CircleVertexBufferBase, dataSize);

            s_Data.CircleShader->bind();
            RenderCommand::drawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
            s_Data.stats.drawCalls++;
        }
        if (s_Data.LineVertexCount)
        {
            s_Data.OutlinePipeline->Bind();
            uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.LineVertexBufferPtr - (uint8_t *)s_Data.LineVertexBufferBase);
            s_Data.LineVertexBuffer->setData(s_Data.LineVertexBufferBase, dataSize);

            s_Data.LineShader->bind();
            RenderCommand::setLineWidth(s_Data.LineWidth);
            RenderCommand::drawLines(s_Data.LineVertexArray, s_Data.LineVertexCount);
            s_Data.stats.drawCalls++;
        }

        if (s_Data.TextIndexCount)
        {
            uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.TextVertexBufferPtr - (uint8_t *)s_Data.TextVertexBufferBase);
            s_Data.TextVertexBuffer->setData(s_Data.TextVertexBufferBase, dataSize);

            if (s_Data.FontAtlasTexture)
                s_Data.FontAtlasTexture->bind(0);

            s_Data.TextShader->bind();
            RenderCommand::drawIndexed(s_Data.TextVertexArray, s_Data.TextIndexCount);
            s_Data.stats.drawCalls++;
        }
    }

    void Renderer2D::flushAndReset()
    {
        endScene();
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.QuadInstanceCount = 0;
        s_Data.QuadInstanceVertexBufferPtr = s_Data.QuadInstanceVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextureSlotIndex = 1;

        s_Data.TextIndexCount = 0;
        s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        drawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(
                                                                              glm::mat4(1.0f), {size.x, size.y, 1.0f});
        drawQuad(transform, color);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size,
                              const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size,
                              const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(
                                                                              glm::mat4(1.0f), {size.x, size.y, 1.0f});

        drawQuad(transform, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size,
                              const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor,
                              glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, subtexture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size,
                              const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor,
                              glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }

        constexpr glm::vec4 color = glm::vec4(1.0f);

        const glm::vec2 *txCoord = subtexture->getTexCoords();
        const std::shared_ptr<Texture2D> &texture = subtexture->getTexture();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(
                                                                              glm::mat4(1.0f), {size.x, size.y, 1.0f});
        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = -1;
            s_Data.QuadVertexBufferPtr++;
        }
        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4 &transform, const glm::vec4 &color, int objectID)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }
        const float textureIndex = 0.0f;
        const float tilingFactor = 1.0f;

        glm::vec2 txCoord[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = objectID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4 &transform, const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                              glm::vec4 tintColor, int objectID)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        const glm::vec2 txCoord[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f},
        };

        const glm::vec4 color = tintColor;

        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = objectID;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4 &transform, const std::shared_ptr<SubTexture2D> &subTexture,
                              float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }
        constexpr glm::vec4 color = glm::vec4(1.0f);

        const glm::vec2 *txCoord = subTexture->getTexCoords();
        const std::shared_ptr<Texture2D> &texture = subTexture->getTexture();

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = -1;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuadBillboard(const glm::vec3 &position,
                                       const glm::vec2 &size,
                                       const glm::vec4 &color, int objectId)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            flushAndReset();

        // ===== 1. 计算 Billboard 旋转 =====
        glm::mat4 view = s_Data.m_CameraView;
        view[3] = glm::vec4(0, 0, 0, 1);
        glm::mat4 billboardRotation = glm::inverse(view);

        // ===== 2. 构造 Model 矩阵 =====
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            billboardRotation *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        constexpr float textureIndex = 0.0f;
        constexpr float tilingFactor = 1.0f;

        constexpr glm::vec2 txCoord[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}};

        for (uint32_t i = 0; i < 4; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = objectId;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuadBillboard(const glm::vec3 &position,
                                       const glm::vec2 &size,
                                       const std::shared_ptr<Texture2D> &texture,
                                       float tilingFactor,
                                       const glm::vec4 &tintColor, int objectId)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            flushAndReset();

        // ===== 1. 计算 Billboard 旋转 =====
        glm::mat4 view = s_Data.m_CameraView;
        view[3] = glm::vec4(0, 0, 0, 1);
        glm::mat4 billboardRotation = glm::inverse(view);

        // ===== 2. Model 矩阵 =====
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            billboardRotation *
            glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        // ===== 3. 纹理槽 =====
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        constexpr glm::vec2 txCoord[4] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}};

        for (uint32_t i = 0; i < 4; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = tintColor;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = objectId;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawAABB(const AABB &aabb, const glm::mat4 &transform, const glm::vec4 &color, int objectId)
    {
        FM_PROFILE_FUNCTION();
        
        glm::vec4 min = {aabb.min.x, aabb.min.y, aabb.min.z, 1.0f};
        glm::vec4 max = {aabb.max.x, aabb.max.y, aabb.max.z, 1.0f};

        glm::vec4 corners[8] =
            {
                transform * glm::vec4{aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
                transform * glm::vec4{aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
                transform * glm::vec4{aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
                transform * glm::vec4{aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},

                transform * glm::vec4{aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
                transform * glm::vec4{aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
                transform * glm::vec4{aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
                transform * glm::vec4{aabb.max.x, aabb.min.y, aabb.min.z, 1.0f}};

        for (uint32_t i = 0; i < 4; i++)
            drawLine(corners[i], corners[(i + 1) % 4], color, objectId);

        for (uint32_t i = 0; i < 4; i++)
            drawLine(corners[i + 4], corners[((i + 1) % 4) + 4], color, objectId);

        for (uint32_t i = 0; i < 4; i++)
            drawLine(corners[i], corners[i + 4], color, objectId);
    }

    void Renderer2D::drawQuadInstanced(const glm::mat4 &transform, const glm::vec4 &color,
                                       const std::shared_ptr<Texture2D> &texture, float tilingFactor, int objectID)
    {
        FM_PROFILE_FUNCTION();
        if (s_Data.QuadInstanceCount >= Renderer2DData::MaxQuads)
            flushAndReset();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        QuadInstanceData instance;
        instance.transform = transform;
        instance.color = color;
        instance.texIndex = textureIndex;
        instance.tilingFactor = tilingFactor;
        instance.objectID = objectID;

        *s_Data.QuadInstanceVertexBufferPtr = instance;
        s_Data.QuadInstanceVertexBufferPtr++;
        s_Data.QuadInstanceCount++;
        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuadInstanced(const glm::mat4 &transform, const glm::vec4 &color, int objectID)
    {
        drawQuadInstanced(transform, color, s_Data.WhiteTexture, 1.0f, objectID);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                     const glm::vec4 &color)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, color);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                     const glm::vec4 &color)
    {
        FM_PROFILE_FUNCTION();

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }
        const float textureIndex = 0.0f;
        const float tilingFactor = 1.0f;
        constexpr glm::vec2 txCoord[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}};

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = -1;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }
        constexpr glm::vec4 color = glm::vec4(1.0f);

        float textureIndex = 0.0f;

        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        constexpr glm::vec2 txCoord[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f}};
        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = -1;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, subtexture, tilingFactor, tintColor);
    }

    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor,
                                     glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();
        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        {
            flushAndReset();
        }
        constexpr glm::vec4 color = glm::vec4(1.0f);

        float textureIndex = 0.0f;
        const std::shared_ptr<Texture2D> &texture = subtexture->getTexture();

        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++)
        {
            if (*s_Data.TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }
        if (textureIndex == 0.0f)
        {
            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        const glm::vec2 *txCoord = subtexture->getTexCoords();
        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr->objectID = -1;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade,
                                int objectID)
    {
        FM_PROFILE_FUNCTION();

        // TODO: implement for circles
        // if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        // 	NextBatch();

        for (size_t i = 0; i < 4; i++)
        {
            s_Data.CircleVertexBufferPtr->worldPosition = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->localPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->color = color;
            s_Data.CircleVertexBufferPtr->thickness = thickness;
            s_Data.CircleVertexBufferPtr->fade = fade;
            s_Data.CircleVertexBufferPtr->objectID = objectID;
            s_Data.CircleVertexBufferPtr++;
        }

        s_Data.CircleIndexCount += 6;

        s_Data.stats.circleCount++;
    }

    void Renderer2D::drawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color, int objectID)
    {
        s_Data.LineVertexBufferPtr->position = p0;
        s_Data.LineVertexBufferPtr->color = color;
        s_Data.LineVertexBufferPtr->objectID = objectID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexBufferPtr->position = p1;
        s_Data.LineVertexBufferPtr->color = color;
        s_Data.LineVertexBufferPtr->objectID = objectID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexCount += 2;
        s_Data.stats.lineCount++;
    }

    void Renderer2D::drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, int objectID)
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

    void Renderer2D::drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectID)
    {
        glm::vec3 lineVertices[4];
        for (size_t i = 0; i < 4; i++)
            lineVertices[i] = transform * s_Data.QuadVertexPositions[i];

        drawLine(lineVertices[0], lineVertices[1], color, objectID);
        drawLine(lineVertices[1], lineVertices[2], color, objectID);
        drawLine(lineVertices[2], lineVertices[3], color, objectID);
        drawLine(lineVertices[3], lineVertices[0], color, objectID);
    }

    float Renderer2D::getLineWidth()
    {
        return s_Data.LineWidth;
    }

    void Renderer2D::setLineWidth(float width)
    {
        s_Data.LineWidth = width;
    }

    void Renderer2D::drawString(const std::string &string, std::shared_ptr<Font> font, const glm::mat4 &transform,
                                const TextParams &textParams, int objectId)
    {
        if (!font)
            return;

        const MSDFData *msdfData = font->getMSDFData();
        if (!msdfData)
            return;

        const auto &fontGeometry = msdfData->fontGeometry;
        const auto &metrics = fontGeometry.getMetrics();

        std::shared_ptr<Texture2D> fontAtlas = font->getAtlasTexture();
        if (!fontAtlas)
            return;

        s_Data.FontAtlasTexture = fontAtlas;

        double x = 0.0;
        double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
        double y = 0.0;

        const msdf_atlas::GlyphGeometry *spaceGlyph = fontGeometry.getGlyph(' ');
        if (!spaceGlyph)
            return;

        const float spaceGlyphAdvance = spaceGlyph->getAdvance();

        for (size_t i = 0; i < string.size(); i++)
        {
            char character = string[i];
            if (character == '\r')
                continue;

            if (character == '\n')
            {
                x = 0;
                y -= fsScale * metrics.lineHeight + textParams.lineSpacing;
                continue;
            }

            if (character == ' ')
            {
                float advance = spaceGlyphAdvance;
                if (i < string.size() - 1)
                {
                    char nextCharacter = string[i + 1];
                    double dAdvance;
                    fontGeometry.getAdvance(dAdvance, character, nextCharacter);
                    advance = (float)dAdvance;
                }

                x += fsScale * advance + textParams.kerning;
                continue;
            }

            if (character == '\t')
            {
                // NOTE(Yan): is this right?
                x += 4.0f * (fsScale * spaceGlyphAdvance + textParams.kerning);
                continue;
            }

            auto glyph = fontGeometry.getGlyph(character);
            if (!glyph)
                glyph = fontGeometry.getGlyph('?');
            if (!glyph)
                return;

            double al, ab, ar, at;
            glyph->getQuadAtlasBounds(al, ab, ar, at);
            glm::vec2 texCoordMin((float)al, (float)ab);
            glm::vec2 texCoordMax((float)ar, (float)at);

            double pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            glm::vec2 quadMin((float)pl, (float)pb);
            glm::vec2 quadMax((float)pr, (float)pt);

            quadMin *= fsScale, quadMax *= fsScale;
            quadMin += glm::vec2(x, y);
            quadMax += glm::vec2(x, y);

            float texelWidth = 1.0f / fontAtlas->getWidth();
            float texelHeight = 1.0f / fontAtlas->getHeight();
            texCoordMin *= glm::vec2(texelWidth, texelHeight);
            texCoordMax *= glm::vec2(texelWidth, texelHeight);

            // render here
            s_Data.TextVertexBufferPtr->position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->color = textParams.color;
            s_Data.TextVertexBufferPtr->texCoord = texCoordMin;
            s_Data.TextVertexBufferPtr->objectID = objectId;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextVertexBufferPtr->position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->color = textParams.color;
            s_Data.TextVertexBufferPtr->texCoord = {texCoordMin.x, texCoordMax.y};
            s_Data.TextVertexBufferPtr->objectID = objectId;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextVertexBufferPtr->position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->color = textParams.color;
            s_Data.TextVertexBufferPtr->texCoord = texCoordMax;
            s_Data.TextVertexBufferPtr->objectID = objectId;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextVertexBufferPtr->position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->color = textParams.color;
            s_Data.TextVertexBufferPtr->texCoord = {texCoordMax.x, texCoordMin.y};
            s_Data.TextVertexBufferPtr->objectID = objectId;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextIndexCount += 6;
            s_Data.stats.quadCount++;

            if (i < string.size() - 1)
            {
                double advance = glyph->getAdvance();
                char nextCharacter = string[i + 1];
                fontGeometry.getAdvance(advance, character, nextCharacter);

                x += fsScale * advance + textParams.kerning;
            }
        }
    }
    void Renderer2D::recordOutlinePass(CommandBuffer &commandBuffer, const std::vector<MeshDrawCommand> &drawCommands)
    {
        const auto *commands = &drawCommands;
        commandBuffer.Record(
            [commands](RendererAPI &)
            {
                for (auto &mesh : *commands)
                {
                    if (mesh.drawOutline)
                    {
                        Renderer2D::drawAABB(mesh.mesh->getBoundingBox(), mesh.transform, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), mesh.objectID);
                    }
                }
            });
    }

    void Renderer2D::resetStatistics()
    {
        memset(&s_Data.stats, 0, sizeof(Renderer2D::Satistics));
    }

    Renderer2D::Satistics Renderer2D::getStatistics()
    {
        return s_Data.stats;
    }
} // namespace Fermion
