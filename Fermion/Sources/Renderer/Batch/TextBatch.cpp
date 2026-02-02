#include "fmpch.hpp"
#include "TextBatch.hpp"
#include "Renderer/Font/Font.hpp"
#include "Renderer/Font/MSDFData.hpp"

namespace Fermion
{
    TextBatch::TextBatch()
        : m_VertexBuffer(MaxVertices)
    {
    }

    void TextBatch::init(std::shared_ptr<IndexBuffer> sharedIndexBuffer)
    {
        m_VertexArray = VertexArray::create();
        m_GPUVertexBuffer = VertexBuffer::create(MaxVertices * sizeof(TextVertex));
        m_GPUVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Int, "a_ObjectID"}
        });
        m_VertexArray->addVertexBuffer(m_GPUVertexBuffer);

        // Use shared index buffer if provided, otherwise create our own
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

    void TextBatch::shutdown()
    {
        m_VertexArray.reset();
        m_GPUVertexBuffer.reset();
        if (m_OwnsIndexBuffer)
            m_IndexBuffer.reset();
        m_FontAtlasTexture.reset();
    }

    void TextBatch::reset()
    {
        m_VertexBuffer.reset();
        m_IndexCount = 0;
        m_FontAtlasTexture.reset();
    }

    void TextBatch::createIndexBuffer()
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

    void TextBatch::submit(const std::string& text, std::shared_ptr<Font> font,
                          const glm::mat4& transform, const TextParams& textParams,
                          int objectId)
    {
        if (!font)
            return;

        const MSDFData* msdfData = font->getMSDFData();
        if (!msdfData)
            return;

        const auto& fontGeometry = msdfData->fontGeometry;
        const auto& metrics = fontGeometry.getMetrics();

        std::shared_ptr<Texture2D> fontAtlas = font->getAtlasTexture();
        if (!fontAtlas)
            return;

        m_FontAtlasTexture = fontAtlas;

        double x = 0.0;
        double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
        double y = 0.0;

        const msdf_atlas::GlyphGeometry* spaceGlyph = fontGeometry.getGlyph(' ');
        if (!spaceGlyph)
            return;

        const float spaceGlyphAdvance = spaceGlyph->getAdvance();

        for (size_t i = 0; i < text.size(); i++)
        {
            char character = text[i];
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
                if (i < text.size() - 1)
                {
                    char nextCharacter = text[i + 1];
                    double dAdvance;
                    fontGeometry.getAdvance(dAdvance, character, nextCharacter);
                    advance = (float)dAdvance;
                }

                x += fsScale * advance + textParams.kerning;
                continue;
            }

            if (character == '\t')
            {
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

            // Submit 4 vertices for this glyph quad
            // Vertex 0: bottom-left
            TextVertex* v0 = m_VertexBuffer.current();
            if (!v0) return;
            v0->position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
            v0->color = textParams.color;
            v0->texCoord = texCoordMin;
            v0->objectID = objectId;
            m_VertexBuffer.advance();

            // Vertex 1: top-left
            TextVertex* v1 = m_VertexBuffer.current();
            if (!v1) return;
            v1->position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
            v1->color = textParams.color;
            v1->texCoord = {texCoordMin.x, texCoordMax.y};
            v1->objectID = objectId;
            m_VertexBuffer.advance();

            // Vertex 2: top-right
            TextVertex* v2 = m_VertexBuffer.current();
            if (!v2) return;
            v2->position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
            v2->color = textParams.color;
            v2->texCoord = texCoordMax;
            v2->objectID = objectId;
            m_VertexBuffer.advance();

            // Vertex 3: bottom-right
            TextVertex* v3 = m_VertexBuffer.current();
            if (!v3) return;
            v3->position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
            v3->color = textParams.color;
            v3->texCoord = {texCoordMax.x, texCoordMin.y};
            v3->objectID = objectId;
            m_VertexBuffer.advance();

            m_IndexCount += 6;

            if (i < text.size() - 1)
            {
                double advance = glyph->getAdvance();
                char nextCharacter = text[i + 1];
                fontGeometry.getAdvance(advance, character, nextCharacter);

                x += fsScale * advance + textParams.kerning;
            }
        }
    }

    void TextBatch::uploadToGPU()
    {
        if (m_VertexBuffer.isEmpty())
            return;

        m_GPUVertexBuffer->setData(m_VertexBuffer.data(), m_VertexBuffer.getDataSize());
    }

    void TextBatch::bindFontAtlas()
    {
        if (m_FontAtlasTexture)
            m_FontAtlasTexture->bind(0);
    }

} // namespace Fermion
