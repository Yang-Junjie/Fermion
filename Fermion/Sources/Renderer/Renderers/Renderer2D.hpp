#pragma once
#include "Renderer/RendererConfig.hpp"
#include "Renderer/Batch/QuadBatch.hpp"
#include "Renderer/Batch/CircleBatch.hpp"
#include "Renderer/Batch/LineBatch.hpp"
#include "Renderer/Batch/TextBatch.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Math/AABB.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace Fermion
{
    class Texture2D;
    class SubTexture2D;
    class OrthographicCamera;
    class Camera;
    class EditorCamera;
    class Font;
    class Pipeline;
    class Shader;
    class UniformBuffer;
    class RenderGraphLegacy;
    class RenderCommandQueue;
    class RendererAPI;


    class Renderer2D
    {
    public:
        Renderer2D();
        ~Renderer2D();

        Renderer2D(const Renderer2D&) = delete;
        Renderer2D& operator=(const Renderer2D&) = delete;


        void init(const RendererConfig& config);


        void shutdown();


        void beginScene(const OrthographicCamera& camera);
        void beginScene(const EditorCamera& camera);
        void beginScene(const Camera& camera, const glm::mat4& view);

        void endScene();
        void flush();



        void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
        void drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

        void drawQuad(const glm::vec2& position, const glm::vec2& size,
                     const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                     glm::vec4 tintColor = glm::vec4(1.0f));
        void drawQuad(const glm::vec3& position, const glm::vec2& size,
                     const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                     glm::vec4 tintColor = glm::vec4(1.0f));

        void drawQuad(const glm::vec2& position, const glm::vec2& size,
                     const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                     glm::vec4 tintColor = glm::vec4(1.0f));
        void drawQuad(const glm::vec3& position, const glm::vec2& size,
                     const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                     glm::vec4 tintColor = glm::vec4(1.0f));

        void drawQuad(const glm::mat4& transform, const glm::vec4& color, int objectId = -1);
        void drawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture,
                     float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f), int objectId = -1);
        void drawQuad(const glm::mat4& transform, const std::shared_ptr<SubTexture2D>& subTexture,
                     float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f));



        void drawQuadBillboard(const glm::vec3& position, const glm::vec2& size,
                              const glm::vec4& color, int objectId = -1);
        void drawQuadBillboard(const glm::vec3& position, const glm::vec2& size,
                              const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                              const glm::vec4& tintColor = glm::vec4(1.0f), int objectId = -1);


        void drawAABB(const AABB& aabb, const glm::mat4& transform,
                     const glm::vec4& color, int objectId = -1);



        void drawQuadInstanced(const glm::mat4& transform, const glm::vec4& color,
                              const std::shared_ptr<Texture2D>& texture, float tilingFactor, int objectID);
        void drawQuadInstanced(const glm::mat4& transform, const glm::vec4& color, int objectID = -1);

        void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                            const glm::vec4& color);
        void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                            const glm::vec4& color);

        void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                            const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f));
        void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                            const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f));

        void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                            const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f));
        void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                            const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f));



        void drawCircle(const glm::mat4& transform, const glm::vec4& color,
                        float thickness = 1.0f, float fade = 0.005f, int objectId = -1);


        void drawLine(const glm::vec3& p0, const glm::vec3& p1,
                     const glm::vec4& color, int objectId = -1);

        void drawRect(const glm::vec3& position, const glm::vec2& size,
                     const glm::vec4& color, int objectId = -1);
        void drawRect(const glm::mat4& transform, const glm::vec4& color, int objectId = -1);

        float getLineWidth();
        void setLineWidth(float width);



        void recordOutlinePass(RenderCommandQueue& queue,
                              const std::vector<MeshDrawCommand>& drawCommands,
                              const glm::vec4& outlineColor);

        using TextParams = Fermion::TextParams;

        void drawString(const std::string& string, std::shared_ptr<Font> font,
                       const glm::mat4& transform, const TextParams& textParams,
                       int objectId = -1);



        struct Satistics
        {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;
            uint32_t lineCount = 0;
            uint32_t circleCount = 0;

            uint32_t getTotalVertexCount()
            {
                return quadCount * 4 + lineCount * 2 + circleCount * 4;
            }

            uint32_t getTotalIndexCount()
            {
                return quadCount * 6 + circleCount * 6;
            }
        };

        void resetStatistics();
        Satistics getStatistics() const;

    private:
        void flushAndReset();

        // Render pass methods
        void quadPass();
        void quadInstancePass();
        void circlePass();
        void linePass();
        void textPass();

        // Camera uniform buffer update
        void updateCameraUBO(const glm::mat4& viewProj, const glm::mat4& view,
                            const glm::vec3& cameraPos);
        void resetBuffers();

    private:
        // Batch renderers
        std::unique_ptr<QuadBatch> m_QuadBatch;
        std::unique_ptr<CircleBatch> m_CircleBatch;
        std::unique_ptr<LineBatch> m_LineBatch;
        std::unique_ptr<TextBatch> m_TextBatch;

        // Pipelines
        std::shared_ptr<Pipeline> m_QuadPipeline;
        std::shared_ptr<Pipeline> m_QuadInstancePipeline;
        std::shared_ptr<Pipeline> m_CirclePipeline;
        std::shared_ptr<Pipeline> m_LinePipeline;
        std::shared_ptr<Pipeline> m_TextPipeline;

        // Shaders
        std::shared_ptr<Shader> m_QuadShader;
        std::shared_ptr<Shader> m_QuadInstanceShader;
        std::shared_ptr<Shader> m_CircleShader;
        std::shared_ptr<Shader> m_LineShader;
        std::shared_ptr<Shader> m_TextShader;

        // Camera state
        std::shared_ptr<UniformBuffer> m_CameraUBO;
        glm::mat4 m_CameraViewProj{1.0f};
        glm::mat4 m_CameraView{1.0f};

        // Render graph
        std::unique_ptr<RenderGraphLegacy> m_RenderGraph;
        std::unique_ptr<RenderCommandQueue> m_CommandQueue;

        // Statistics
        Satistics m_Stats;
    };

} // namespace Fermion
