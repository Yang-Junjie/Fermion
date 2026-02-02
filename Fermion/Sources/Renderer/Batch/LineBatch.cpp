#include "fmpch.hpp"
#include "LineBatch.hpp"

namespace Fermion
{
    LineBatch::LineBatch()
        : m_VertexBuffer(MaxVertices)
    {
    }

    void LineBatch::init()
    {
        m_VertexArray = VertexArray::create();
        m_GPUVertexBuffer = VertexBuffer::create(MaxVertices * sizeof(LineVertex));
        m_GPUVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Int, "a_ObjectID"}
        });
        m_VertexArray->addVertexBuffer(m_GPUVertexBuffer);
        // No index buffer for lines
    }

    void LineBatch::shutdown()
    {
        m_VertexArray.reset();
        m_GPUVertexBuffer.reset();
    }

    void LineBatch::reset()
    {
        m_VertexBuffer.reset();
        m_VertexCount = 0;
    }

    void LineBatch::submit(const glm::vec3& p0, const glm::vec3& p1,
                          const glm::vec4& color, int objectID)
    {
        // First vertex
        LineVertex* v0 = m_VertexBuffer.current();
        if (!v0)
            return; // Buffer full

        v0->position = p0;
        v0->color = color;
        v0->objectID = objectID;
        m_VertexBuffer.advance();

        // Second vertex
        LineVertex* v1 = m_VertexBuffer.current();
        if (!v1)
            return; // Buffer full

        v1->position = p1;
        v1->color = color;
        v1->objectID = objectID;
        m_VertexBuffer.advance();

        m_VertexCount += 2;
    }

    void LineBatch::uploadToGPU()
    {
        if (m_VertexBuffer.isEmpty())
            return;

        m_GPUVertexBuffer->setData(m_VertexBuffer.data(), m_VertexBuffer.getDataSize());
    }

} // namespace Fermion
