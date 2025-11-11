#include "fmpch.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "Renderer2D.hpp"
namespace Fermion
{
    struct QuadVertices
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 txCoord;
        float texIndex;
        float tilingFactor;
    };
    struct Renderer2DData
    {
        static const uint32_t MaxQuads = 10000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32;

        std::shared_ptr<VertexArray> QuadVertexArray;
        std::shared_ptr<VertexBuffer> QuadVertexBuffer;
        std::shared_ptr<Shader> TextureShader;
        std::shared_ptr<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertices *QuadVertexBufferBase = nullptr;
        QuadVertices *QuadVertexBufferPtr = nullptr;

        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];

        Renderer2D::Satistics stats;
    };

    static Renderer2DData s_Data;

    void Renderer2D::init()
    {
        FM_PROFILE_FUNCTION();

        s_Data.QuadVertexArray = VertexArray::create();

        s_Data.QuadVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(QuadVertices));
        s_Data.QuadVertexBuffer->setLayout({{ShaderDataType::Float3, "a_Position"},
                                            {ShaderDataType::Float4, "a_Color"},
                                            {ShaderDataType::Float2, "a_TexCoord"},
                                            {ShaderDataType::Float, "a_TexIndex"},
                                            {ShaderDataType::Float, "a_TilingFactor"}});
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

        s_Data.WhiteTexture = Texture2D::create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->setData(&whiteTextureData, sizeof(uint32_t));

        s_Data.TextureShader = Shader::create("../game/assets/shaders/Texture.glsl");
        s_Data.TextureShader->bind();
        int samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;
        s_Data.TextureShader->setIntArray("u_Textures", samplers, s_Data.MaxTextureSlots);

        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    }

    void Renderer2D::shutdown()
    {
        FM_PROFILE_FUNCTION();
    }

    void Renderer2D::beginScene(const OrthographicCamera &camera)
    {
        FM_PROFILE_FUNCTION();

        s_Data.TextureShader->bind();
        s_Data.TextureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.QuadIndexCount = 0;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::beginScene(const Camera &camera, const glm::mat4 &Transform)
    {

        FM_PROFILE_FUNCTION();

        glm::mat4 viewProj = camera.getProjection() * glm::inverse(Transform);

        s_Data.TextureShader->bind();
        s_Data.TextureShader->setMat4("u_ViewProjection", viewProj);
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.QuadIndexCount = 0;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::endScene()
    {
        FM_PROFILE_FUNCTION();
        if (s_Data.QuadIndexCount == 0)
            return;
        uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.QuadVertexBufferPtr - (uint8_t *)s_Data.QuadVertexBufferBase);
        if (dataSize > 0)
            s_Data.QuadVertexBuffer->setData(s_Data.QuadVertexBufferBase, dataSize);
        flush();
    }

    void Renderer2D::flush()
    {
        FM_PROFILE_FUNCTION();
        if (s_Data.QuadIndexCount == 0)
            return;
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
        {
            s_Data.TextureSlots[i]->bind(i);
        }
        RenderCommand::drawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
        s_Data.stats.drawCalls++;
    }

    void Renderer2D::flushAndReset()
    {
        endScene();
        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;
        s_Data.TextureSlotIndex = 1;
    }
    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        drawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        drawQuad(transform, color);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        drawQuad(transform, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor, glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, subtexture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor, glm::vec4 tintColor)
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

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }
        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }
    void Renderer2D::drawQuad(const glm::mat4 &transform, const glm::vec4 &color)
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
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4 &transform, const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
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
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;
        s_Data.stats.quadCount++;
    }

    void Renderer2D::drawQuad(const glm::mat4 &transform, const std::shared_ptr<SubTexture2D> &subTexture, float tilingFactor, glm::vec4 tintColor)
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
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }
    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians, const glm::vec4 &color)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, color);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians, const glm::vec4 &color)
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

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }
    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, texture, tilingFactor, tintColor);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
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

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

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
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }
    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor, glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radians, subtexture, tilingFactor, tintColor);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians,
                                     const std::shared_ptr<SubTexture2D> &subtexture, float tilingFactor, glm::vec4 tintColor)
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

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::rotate(glm::mat4(1.0f), radians, {0.0f, 0.0f, 1.0f}) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        const glm::vec2 *txCoord = subtexture->getTexCoords();
        const uint32_t vertexCount = 4;
        for (uint32_t i = 0; i < vertexCount; i++)
        {
            s_Data.QuadVertexBufferPtr->position = transform * s_Data.QuadVertexPositions[i];
            s_Data.QuadVertexBufferPtr->color = color;
            s_Data.QuadVertexBufferPtr->txCoord = txCoord[i];
            s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data.QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data.QuadVertexBufferPtr++;
        }

        s_Data.QuadIndexCount += 6;

        s_Data.stats.quadCount++;
    }
    void Renderer2D::resetStatistics()
    {
        memset(&s_Data.stats, 0, sizeof(Renderer2D::Satistics));
    }
    Renderer2D::Satistics Renderer2D::getStatistics()
    {
        return s_Data.stats;
    }
}
