#include "fmpch.hpp"
#include "QuadBatch.hpp"

namespace Fermion
{
    QuadBatch::QuadBatch()
        : m_VertexBuffer(MaxVertices)
        , m_InstanceBuffer(MaxQuads)
    {
        // Initialize quad vertex positions (unit quad centered at origin)
        m_QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        m_QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        m_QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        m_QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};
    }

    void QuadBatch::init()
    {
        // Create vertex array and buffer for regular quads
        m_VertexArray = VertexArray::create();
        m_GPUVertexBuffer = VertexBuffer::create(MaxVertices * sizeof(QuadVertex));
        m_GPUVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float, "a_TexIndex"},
            {ShaderDataType::Float, "a_TilingFactor"},
            {ShaderDataType::Int, "a_ObjectID"}
        });
        m_VertexArray->addVertexBuffer(m_GPUVertexBuffer);

        // Create shared index buffer
        createIndexBuffer();
        m_VertexArray->setIndexBuffer(m_IndexBuffer);

        // Create vertex array and buffer for instanced quads
        m_InstanceVertexArray = VertexArray::create();
        m_InstanceGPUVertexBuffer = VertexBuffer::create(MaxQuads * sizeof(QuadInstanceData));
        m_InstanceGPUVertexBuffer->setLayout({
            {ShaderDataType::Mat4, "a_Transform", false, 1},
            {ShaderDataType::Float4, "a_Color", false, 1},
            {ShaderDataType::Float, "a_TexIndex", false, 1},
            {ShaderDataType::Float, "a_TilingFactor", false, 1},
            {ShaderDataType::Int, "a_ObjectID", false, 1}
        });
        m_InstanceVertexArray->addVertexBuffer(m_InstanceGPUVertexBuffer);
        m_InstanceVertexArray->setIndexBuffer(m_IndexBuffer);

        // Create white texture for solid color rendering
        m_WhiteTexture = Texture2D::create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        m_WhiteTexture->setData(&whiteTextureData, sizeof(uint32_t));

        // Set white texture in slot 0
        m_TextureSlots[0] = m_WhiteTexture;
    }

    void QuadBatch::shutdown()
    {
        m_VertexArray.reset();
        m_GPUVertexBuffer.reset();
        m_IndexBuffer.reset();
        m_InstanceVertexArray.reset();
        m_InstanceGPUVertexBuffer.reset();
        m_WhiteTexture.reset();

        for (auto& slot : m_TextureSlots)
            slot.reset();
    }

    void QuadBatch::reset()
    {
        m_VertexBuffer.reset();
        m_InstanceBuffer.reset();
        m_IndexCount = 0;
        m_TextureSlotIndex = 1; // Reset to 1, keep white texture in slot 0
    }

    void QuadBatch::createIndexBuffer()
    {
        std::vector<uint32_t> indices(MaxIndices);
        uint32_t offset = 0;

        for (uint32_t i = 0; i < MaxIndices; i += 6)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;

            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            offset += 4;
        }

        m_IndexBuffer = IndexBuffer::create(indices.data(), MaxIndices);
    }

    void QuadBatch::submit(const glm::mat4& transform, const glm::vec4& color,
                          const glm::vec2* texCoords, float texIndex,
                          float tilingFactor, int objectID)
    {
        for (uint32_t i = 0; i < QuadVertexCount; i++)
        {
            QuadVertex* vertex = m_VertexBuffer.current();
            if (!vertex)
                return; // Buffer full

            vertex->position = transform * m_QuadVertexPositions[i];
            vertex->color = color;
            vertex->texCoord = texCoords[i];
            vertex->texIndex = texIndex;
            vertex->tilingFactor = tilingFactor;
            vertex->objectID = objectID;

            m_VertexBuffer.advance();
        }

        m_IndexCount += 6;
    }

    void QuadBatch::submitInstanced(const glm::mat4& transform, const glm::vec4& color,
                                   float texIndex, float tilingFactor, int objectID)
    {
        QuadInstanceData* instance = m_InstanceBuffer.current();
        if (!instance)
            return; // Buffer full

        instance->transform = transform;
        instance->color = color;
        instance->texIndex = texIndex;
        instance->tilingFactor = tilingFactor;
        instance->objectID = objectID;

        m_InstanceBuffer.advance();
    }

    float QuadBatch::getTextureIndex(const std::shared_ptr<Texture2D>& texture)
    {
        // Search for existing texture
        for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
        {
            if (*m_TextureSlots[i].get() == *texture.get())
                return static_cast<float>(i);
        }

        // Allocate new slot
        float index = static_cast<float>(m_TextureSlotIndex);
        m_TextureSlots[m_TextureSlotIndex] = texture;
        m_TextureSlotIndex++;

        return index;
    }

    void QuadBatch::uploadToGPU()
    {
        if (m_VertexBuffer.isEmpty())
            return;

        m_GPUVertexBuffer->setData(m_VertexBuffer.data(), m_VertexBuffer.getDataSize());
    }

    void QuadBatch::uploadInstanceDataToGPU()
    {
        if (m_InstanceBuffer.isEmpty())
            return;

        m_InstanceGPUVertexBuffer->setData(m_InstanceBuffer.data(), m_InstanceBuffer.getDataSize());
    }

    void QuadBatch::bindTextures()
    {
        for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
        {
            if (m_TextureSlots[i])
                m_TextureSlots[i]->bind(i);
        }
    }

    bool QuadBatch::isFull() const
    {
        return m_IndexCount >= MaxIndices;
    }

    bool QuadBatch::isInstanceFull() const
    {
        return m_InstanceBuffer.isFull();
    }

} // namespace Fermion
