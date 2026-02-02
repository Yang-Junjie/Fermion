#include "fmpch.hpp"
#include "CircleBatch.hpp"

namespace Fermion
{
    CircleBatch::CircleBatch()
        : m_VertexBuffer(MaxVertices)
    {
    }

    void CircleBatch::init(std::shared_ptr<IndexBuffer> sharedIndexBuffer)
    {
        m_VertexArray = VertexArray::create();
        m_GPUVertexBuffer = VertexBuffer::create(MaxVertices * sizeof(CircleVertex));
        m_GPUVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_WorldPosition"},
            {ShaderDataType::Float3, "a_LocalPosition"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float, "a_Thickness"},
            {ShaderDataType::Float, "a_Fade"},
            {ShaderDataType::Int, "a_ObjectID"}
        });
        m_VertexArray->addVertexBuffer(m_GPUVertexBuffer);

      
        if (sharedIndexBuffer)
        {
            m_IndexBuffer = sharedIndexBuffer;
            m_OwnsIndexBuffer = false;
        }
        else
        {
            createIndexBuffer();
            m_OwnsIndexBuffer = true;
        }
        m_VertexArray->setIndexBuffer(m_IndexBuffer);
    }

    void CircleBatch::shutdown()
    {
        m_VertexArray.reset();
        m_GPUVertexBuffer.reset();
        if (m_OwnsIndexBuffer)
            m_IndexBuffer.reset();
    }

    void CircleBatch::reset()
    {
        m_VertexBuffer.reset();
        m_IndexCount = 0;
    }

    void CircleBatch::createIndexBuffer()
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

    void CircleBatch::submit(const glm::mat4& transform, const glm::vec4& color,
                            float thickness, float fade, int objectID,
                            const glm::vec4* quadVertexPositions)
    {
        for (uint32_t i = 0; i < 4; i++)
        {
            CircleVertex* vertex = m_VertexBuffer.current();
            if (!vertex)
                return; // Buffer full

            vertex->worldPosition = transform * quadVertexPositions[i];
            vertex->localPosition = quadVertexPositions[i] * 2.0f;
            vertex->color = color;
            vertex->thickness = thickness;
            vertex->fade = fade;
            vertex->objectID = objectID;

            m_VertexBuffer.advance();
        }

        m_IndexCount += 6;
    }

    void CircleBatch::uploadToGPU()
    {
        if (m_VertexBuffer.isEmpty())
            return;

        m_GPUVertexBuffer->setData(m_VertexBuffer.data(), m_VertexBuffer.getDataSize());
    }

} // namespace Fermion
