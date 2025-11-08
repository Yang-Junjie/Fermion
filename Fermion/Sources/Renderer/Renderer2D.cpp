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
    };
    struct Renderer2DData
    {
        uint32_t MaxQuads = 10000;
        uint32_t MaxVertices = MaxQuads * 4;
        uint32_t MaxIndices = MaxQuads * 6;
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
                                            {ShaderDataType::Float, "a_TexIndex"}});
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

    void Renderer2D::endScene()
    {
        FM_PROFILE_FUNCTION();
        uint32_t dataSize = (uint32_t)((uint8_t *)s_Data.QuadVertexBufferPtr - (uint8_t *)s_Data.QuadVertexBufferBase);
        s_Data.QuadVertexBuffer->setData(s_Data.QuadVertexBufferBase, dataSize);
        flush();
    }

    void Renderer2D::flush()
    {
        FM_PROFILE_FUNCTION();
        for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
        {
            s_Data.TextureSlots[i]->bind(i);
        }
        RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        drawQuad(glm::vec3(position, 0.0f), size, color);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        FM_PROFILE_FUNCTION();
        const float textureIndex = 0.0f;
        s_Data.QuadVertexBufferPtr->position = position;
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {0.0f, 0.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->position = {position.x + size.x, position.y, position.z};
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {1.0f, 0.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->position = {position.x + size.x, position.y + size.y, position.z};
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {1.0f, 1.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->position = {position.x, position.y + size.y, position.z};
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {0.0f, 1.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadIndexCount += 6;

        // s_Data.TextureShader->setFloat4("u_Color", color);
        // s_Data.TextureShader->setFloat("u_TilingFactor", 1.0f);
        // s_Data.WhiteTexture->bind();

        // glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        // s_Data.TextureShader->setMat4("u_Transform", transform);

        // s_Data.QuadVertexArray->bind();
        // RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }

    void Renderer2D::drawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        drawQuad(glm::vec3(position, 0.0f), size, texture, tilingFactor, tintColor);
    }

    void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();

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

        s_Data.QuadVertexBufferPtr->position = position;
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {0.0f, 0.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->position = {position.x + size.x, position.y, position.z};
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {1.0f, 0.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->position = {position.x + size.x, position.y + size.y, position.z};
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {1.0f, 1.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadVertexBufferPtr->position = {position.x, position.y + size.y, position.z};
        s_Data.QuadVertexBufferPtr->color = color;
        s_Data.QuadVertexBufferPtr->txCoord = {0.0f, 1.0f};
        s_Data.QuadVertexBufferPtr->texIndex = textureIndex;
        s_Data.QuadVertexBufferPtr++;

        s_Data.QuadIndexCount += 6;

        // s_Data.TextureShader->setFloat4("u_Color", tintColor);
        // s_Data.TextureShader->setFloat("u_TilingFactor", tilingFactor);
        // texture->bind();

        // glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        // s_Data.TextureShader->setMat4("u_Transform", transform);

        // s_Data.QuadVertexArray->bind();
        // RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radian, const glm::vec4 &color)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radian, color);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radian, const glm::vec4 &color)
    {
        FM_PROFILE_FUNCTION();

        s_Data.TextureShader->setFloat4("u_Color", color);
        s_Data.TextureShader->setFloat("u_TilingFactor", 1.0f);
        s_Data.WhiteTexture->bind();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::rotate(glm::mat4(1.0f), radian, {0.0f, 0.0f, 1.0f}) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        s_Data.TextureShader->setMat4("u_Transform", transform);

        s_Data.QuadVertexArray->bind();
        RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radian,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        drawRotatedQuad(glm::vec3(position, 0.0f), size, radian, texture, tilingFactor, tintColor);
    }
    void Renderer2D::drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radian,
                                     const std::shared_ptr<Texture2D> &texture, float tilingFactor, glm::vec4 tintColor)
    {
        FM_PROFILE_FUNCTION();

        s_Data.TextureShader->setFloat4("u_Color", tintColor);
        s_Data.TextureShader->setFloat("u_TilingFactor", tilingFactor);
        texture->bind();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                              glm::rotate(glm::mat4(1.0f), radian, {0.0f, 0.0f, 1.0f}) *
                              glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});
        s_Data.TextureShader->setMat4("u_Transform", transform);

        s_Data.QuadVertexArray->bind();
        RenderCommand::drawIndexed(s_Data.QuadVertexArray);
    }
}
