#pragma once
#include "Renderer/Batch/BatchBuffer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexArray.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace Fermion
{

    struct CircleVertex
    {
        glm::vec3 worldPosition;
        glm::vec3 localPosition;
        glm::vec4 color;
        float thickness;
        float fade;
        int objectID;
    };
    class CircleBatch
    {
    public:
        static constexpr uint32_t MaxCircles = 10000;
        static constexpr uint32_t MaxVertices = MaxCircles * 4;
        static constexpr uint32_t MaxIndices = MaxCircles * 6;

    public:
        CircleBatch();
        ~CircleBatch() = default;

        
        CircleBatch(const CircleBatch&) = delete;
        CircleBatch& operator=(const CircleBatch&) = delete;

       
        void init(std::shared_ptr<IndexBuffer> sharedIndexBuffer = nullptr);

        void shutdown();

        void reset();

        void submit(const glm::mat4& transform, const glm::vec4& color,
                   float thickness, float fade, int objectID,
                   const glm::vec4* quadVertexPositions);

        
        void uploadToGPU();

        bool hasData() const { return m_IndexCount > 0; }

       
        bool isFull() const { return m_IndexCount >= MaxIndices; }

       
        uint32_t getIndexCount() const { return m_IndexCount; }

        std::shared_ptr<VertexArray> getVertexArray() const { return m_VertexArray; }

    private:
        void createIndexBuffer();

    private:
        BatchBuffer<CircleVertex> m_VertexBuffer;
        uint32_t m_IndexCount = 0;

        // GPU resources
        std::shared_ptr<VertexArray> m_VertexArray;
        std::shared_ptr<VertexBuffer> m_GPUVertexBuffer;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
        bool m_OwnsIndexBuffer = false;
    };

} // namespace Fermion
