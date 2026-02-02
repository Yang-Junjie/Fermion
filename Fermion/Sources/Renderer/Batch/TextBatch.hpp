#pragma once
#include "Renderer/Batch/BatchBuffer.hpp"
#include "Renderer/Buffer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Texture/Texture.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Fermion
{
    class Font;

    struct TextVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        int objectID;
    };

    struct TextParams
    {
        glm::vec4 color{1.0f};
        float kerning = 0.0f;
        float lineSpacing = 0.0f;
    };


    class TextBatch
    {
    public:
        static constexpr uint32_t MaxTextQuads = 10000;
        static constexpr uint32_t MaxVertices = MaxTextQuads * 4;
        static constexpr uint32_t MaxIndices = MaxTextQuads * 6;

    public:
        TextBatch();
        ~TextBatch() = default;

        TextBatch(const TextBatch&) = delete;
        TextBatch& operator=(const TextBatch&) = delete;


        void init(std::shared_ptr<IndexBuffer> sharedIndexBuffer = nullptr);

        void shutdown();

        void reset();

        void submit(const std::string& text, std::shared_ptr<Font> font,
                   const glm::mat4& transform, const TextParams& textParams,
                   int objectId);

  
        void uploadToGPU();

  
        void bindFontAtlas();

        bool hasData() const { return m_IndexCount > 0; }

        bool isFull() const { return m_IndexCount >= MaxIndices; }

        uint32_t getIndexCount() const { return m_IndexCount; }

        std::shared_ptr<VertexArray> getVertexArray() const { return m_VertexArray; }

    private:
        void createIndexBuffer();

    private:
        BatchBuffer<TextVertex> m_VertexBuffer;
        uint32_t m_IndexCount = 0;

        // GPU resources
        std::shared_ptr<VertexArray> m_VertexArray;
        std::shared_ptr<VertexBuffer> m_GPUVertexBuffer;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
        bool m_OwnsIndexBuffer = false;

        // Font atlas texture
        std::shared_ptr<Texture2D> m_FontAtlasTexture;
    };

} // namespace Fermion
