#pragma once
#include "Renderer/Batch/BatchBuffer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexArray.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace Fermion
{
   
    struct LineVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        int objectID;
    };

    
    class LineBatch
    {
    public:
        static constexpr uint32_t MaxLines = 10000;
        static constexpr uint32_t MaxVertices = MaxLines * 2;

    public:
        LineBatch();
        ~LineBatch() = default;

        LineBatch(const LineBatch&) = delete;
        LineBatch& operator=(const LineBatch&) = delete;

       
        void init();

    
        void shutdown();


        void reset();
        void submit(const glm::vec3& p0, const glm::vec3& p1,
                   const glm::vec4& color, int objectID);


        void uploadToGPU();

    
        bool hasData() const { return m_VertexCount > 0; }

        bool isFull() const { return m_VertexCount >= MaxVertices; }

        uint32_t getVertexCount() const { return m_VertexCount; }

        std::shared_ptr<VertexArray> getVertexArray() const { return m_VertexArray; }

        float getLineWidth() const { return m_LineWidth; }
        void setLineWidth(float width) { m_LineWidth = width; }

    private:
        BatchBuffer<LineVertex> m_VertexBuffer;
        uint32_t m_VertexCount = 0;

        // GPU resources
        std::shared_ptr<VertexArray> m_VertexArray;
        std::shared_ptr<VertexBuffer> m_GPUVertexBuffer;

        // Line rendering settings
        float m_LineWidth = 2.0f;
    };

} // namespace Fermion
