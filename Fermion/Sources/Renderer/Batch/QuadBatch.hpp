#pragma once
#include "Renderer/Batch/BatchBuffer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Texture/Texture.hpp"
#include <glm/glm.hpp>
#include <array>
#include <memory>

namespace Fermion
{

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        float texIndex;
        float tilingFactor;
        int objectID;
    };


    struct QuadInstanceData
    {
        glm::mat4 transform;
        glm::vec4 color;
        float texIndex;
        float tilingFactor;
        int objectID;
    };

    class QuadBatch
    {
    public:
        static constexpr uint32_t MaxQuads = 10000;
        static constexpr uint32_t MaxVertices = MaxQuads * 4;
        static constexpr uint32_t MaxIndices = MaxQuads * 6;
        static constexpr uint32_t MaxTextureSlots = 32;
        static constexpr uint32_t QuadVertexCount = 4;

        static constexpr glm::vec2 DefaultTexCoords[QuadVertexCount] = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
        };

    public:
        QuadBatch();
        ~QuadBatch() = default;

        // Non-copyable
        QuadBatch(const QuadBatch&) = delete;
        QuadBatch& operator=(const QuadBatch&) = delete;


        void init();

        void shutdown();

        void reset();

        void submit(const glm::mat4& transform, const glm::vec4& color,
                   const glm::vec2* texCoords, float texIndex,
                   float tilingFactor, int objectID);

        void submitInstanced(const glm::mat4& transform, const glm::vec4& color,
                            float texIndex, float tilingFactor, int objectID);

        float getTextureIndex(const std::shared_ptr<Texture2D>& texture);

        void uploadToGPU();

        void uploadInstanceDataToGPU();

        void bindTextures();

        bool hasData() const { return m_VertexBuffer.getCount() > 0; }
        bool hasInstanceData() const { return m_InstanceBuffer.getCount() > 0; }

        bool isFull() const;
        bool isInstanceFull() const;


        uint32_t getIndexCount() const { return m_IndexCount; }
        uint32_t getInstanceCount() const { return m_InstanceBuffer.getCount(); }

        std::shared_ptr<VertexArray> getVertexArray() const { return m_VertexArray; }
        std::shared_ptr<VertexArray> getInstanceVertexArray() const { return m_InstanceVertexArray; }
        std::shared_ptr<IndexBuffer> getIndexBuffer() const { return m_IndexBuffer; }

        const glm::vec4* getQuadVertexPositions() const { return m_QuadVertexPositions; }

        std::shared_ptr<Texture2D> getWhiteTexture() const { return m_WhiteTexture; }

    private:
        void createIndexBuffer();

    private:
        // Vertex batch
        BatchBuffer<QuadVertex> m_VertexBuffer;
        uint32_t m_IndexCount = 0;

        // Instance batch
        BatchBuffer<QuadInstanceData> m_InstanceBuffer;

        // GPU resources
        std::shared_ptr<VertexArray> m_VertexArray;
        std::shared_ptr<VertexBuffer> m_GPUVertexBuffer;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;

        // Instance GPU resources
        std::shared_ptr<VertexArray> m_InstanceVertexArray;
        std::shared_ptr<VertexBuffer> m_InstanceGPUVertexBuffer;

        // Texture management
        std::array<std::shared_ptr<Texture2D>, MaxTextureSlots> m_TextureSlots;
        uint32_t m_TextureSlotIndex = 1; // 0 = white texture

        // White texture for solid colors
        std::shared_ptr<Texture2D> m_WhiteTexture;

        // Quad vertex positions (unit quad centered at origin)
        glm::vec4 m_QuadVertexPositions[4];
    };

} // namespace Fermion
