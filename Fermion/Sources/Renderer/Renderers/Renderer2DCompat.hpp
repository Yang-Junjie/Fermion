#pragma once
#include "Renderer/Renderers/Renderer2D.hpp"
#include <memory>

namespace Fermion
{

    namespace Renderer2DCompat
    {

        inline std::unique_ptr<Renderer2D> g_Instance;
        inline void init(const RendererConfig& config)
        {
            g_Instance = std::make_unique<Renderer2D>();
            g_Instance->init(config);
        }

        inline void shutdown()
        {
            if (g_Instance)
            {
                g_Instance->shutdown();
                g_Instance.reset();
            }
        }


        inline void beginScene(const OrthographicCamera& camera)
        {
            g_Instance->beginScene(camera);
        }

        inline void beginScene(const EditorCamera& camera)
        {
            g_Instance->beginScene(camera);
        }

        inline void beginScene(const Camera& camera, const glm::mat4& view)
        {
            g_Instance->beginScene(camera, view);
        }

        inline void endScene()
        {
            g_Instance->endScene();
        }

        inline void flush()
        {
            g_Instance->flush();
        }


        inline void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
        {
            g_Instance->drawQuad(position, size, color);
        }

        inline void drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
        {
            g_Instance->drawQuad(position, size, color);
        }

        inline void drawQuad(const glm::vec2& position, const glm::vec2& size,
                            const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawQuad(position, size, texture, tilingFactor, tintColor);
        }

        inline void drawQuad(const glm::vec3& position, const glm::vec2& size,
                            const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawQuad(position, size, texture, tilingFactor, tintColor);
        }

        inline void drawQuad(const glm::vec2& position, const glm::vec2& size,
                            const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawQuad(position, size, subTexture, tilingFactor, tintColor);
        }

        inline void drawQuad(const glm::vec3& position, const glm::vec2& size,
                            const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                            glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawQuad(position, size, subTexture, tilingFactor, tintColor);
        }

        inline void drawQuad(const glm::mat4& transform, const glm::vec4& color, int objectId = -1)
        {
            g_Instance->drawQuad(transform, color, objectId);
        }

        inline void drawQuad(const glm::mat4& transform, const std::shared_ptr<Texture2D>& texture,
                            float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f), int objectId = -1)
        {
            g_Instance->drawQuad(transform, texture, tilingFactor, tintColor, objectId);
        }

        inline void drawQuad(const glm::mat4& transform, const std::shared_ptr<SubTexture2D>& subTexture,
                            float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawQuad(transform, subTexture, tilingFactor, tintColor);
        }


        inline void drawQuadBillboard(const glm::vec3& position, const glm::vec2& size,
                                     const glm::vec4& color, int objectId = -1)
        {
            g_Instance->drawQuadBillboard(position, size, color, objectId);
        }

        inline void drawQuadBillboard(const glm::vec3& position, const glm::vec2& size,
                                     const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                                     const glm::vec4& tintColor = glm::vec4(1.0f), int objectId = -1)
        {
            g_Instance->drawQuadBillboard(position, size, texture, tilingFactor, tintColor, objectId);
        }

        inline void drawAABB(const AABB& aabb, const glm::mat4& transform,
                            const glm::vec4& color, int objectId = -1)
        {
            g_Instance->drawAABB(aabb, transform, color, objectId);
        }

        inline void drawQuadInstanced(const glm::mat4& transform, const glm::vec4& color,
                                     const std::shared_ptr<Texture2D>& texture, float tilingFactor, int objectID)
        {
            g_Instance->drawQuadInstanced(transform, color, texture, tilingFactor, objectID);
        }

        inline void drawQuadInstanced(const glm::mat4& transform, const glm::vec4& color, int objectID = -1)
        {
            g_Instance->drawQuadInstanced(transform, color, objectID);
        }

        inline void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                                   const glm::vec4& color)
        {
            g_Instance->drawRotatedQuad(position, size, radians, color);
        }

        inline void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                                   const glm::vec4& color)
        {
            g_Instance->drawRotatedQuad(position, size, radians, color);
        }

        inline void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                                   const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                                   glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawRotatedQuad(position, size, radians, texture, tilingFactor, tintColor);
        }

        inline void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                                   const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f,
                                   glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawRotatedQuad(position, size, radians, texture, tilingFactor, tintColor);
        }

        inline void drawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float radians,
                                   const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                                   glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawRotatedQuad(position, size, radians, subTexture, tilingFactor, tintColor);
        }

        inline void drawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float radians,
                                   const std::shared_ptr<SubTexture2D>& subTexture, float tilingFactor = 1.0f,
                                   glm::vec4 tintColor = glm::vec4(1.0f))
        {
            g_Instance->drawRotatedQuad(position, size, radians, subTexture, tilingFactor, tintColor);
        }

        inline void drawCircle(const glm::mat4& transform, const glm::vec4& color,
                              float thickness = 1.0f, float fade = 0.005f, int objectId = -1)
        {
            g_Instance->drawCircle(transform, color, thickness, fade, objectId);
        }

        inline void drawLine(const glm::vec3& p0, const glm::vec3& p1,
                            const glm::vec4& color, int objectId = -1)
        {
            g_Instance->drawLine(p0, p1, color, objectId);
        }

        inline void drawRect(const glm::vec3& position, const glm::vec2& size,
                            const glm::vec4& color, int objectId = -1)
        {
            g_Instance->drawRect(position, size, color, objectId);
        }

        inline void drawRect(const glm::mat4& transform, const glm::vec4& color, int objectId = -1)
        {
            g_Instance->drawRect(transform, color, objectId);
        }

        inline float getLineWidth()
        {
            return g_Instance->getLineWidth();
        }

        inline void setLineWidth(float width)
        {
            g_Instance->setLineWidth(width);
        }

        inline void recordOutlinePass(RenderCommandQueue& queue,
                                     const std::vector<MeshDrawCommand>& drawCommands,
                                     const glm::vec4& outlineColor)
        {
            g_Instance->recordOutlinePass(queue, drawCommands, outlineColor);
        }

        using TextParams = Renderer2D::TextParams;

        inline void drawString(const std::string& string, std::shared_ptr<Font> font,
                              const glm::mat4& transform, const TextParams& textParams,
                              int objectId = -1)
        {
            g_Instance->drawString(string, font, transform, textParams, objectId);
        }

        using Satistics = Renderer2D::Satistics;

        inline void resetStatistics()
        {
            g_Instance->resetStatistics();
        }

        inline Satistics getStatistics()
        {
            return g_Instance->getStatistics();
        }

        // --- Access to instance ---

        inline Renderer2D* getInstance()
        {
            return g_Instance.get();
        }

    } // namespace Renderer2DCompat


} // namespace Fermion

#define FM_USE_RENDERER2D_COMPAT \
    namespace Fermion { \
        struct Renderer2DStaticProxy { \
            static void init(const RendererConfig& config) { Renderer2DCompat::init(config); } \
            static void shutdown() { Renderer2DCompat::shutdown(); } \
            static void beginScene(const OrthographicCamera& camera) { Renderer2DCompat::beginScene(camera); } \
            static void beginScene(const EditorCamera& camera) { Renderer2DCompat::beginScene(camera); } \
            static void beginScene(const Camera& camera, const glm::mat4& view) { Renderer2DCompat::beginScene(camera, view); } \
            static void endScene() { Renderer2DCompat::endScene(); } \
            static void flush() { Renderer2DCompat::flush(); } \
            static void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) { Renderer2DCompat::drawQuad(position, size, color); } \
            static void drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) { Renderer2DCompat::drawQuad(position, size, color); } \
            static void drawQuad(const glm::mat4& transform, const glm::vec4& color, int objectId = -1) { Renderer2DCompat::drawQuad(transform, color, objectId); } \
            static void drawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int objectId = -1) { Renderer2DCompat::drawCircle(transform, color, thickness, fade, objectId); } \
            static void drawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int objectId = -1) { Renderer2DCompat::drawLine(p0, p1, color, objectId); } \
            static float getLineWidth() { return Renderer2DCompat::getLineWidth(); } \
            static void setLineWidth(float width) { Renderer2DCompat::setLineWidth(width); } \
            static void resetStatistics() { Renderer2DCompat::resetStatistics(); } \
            static Renderer2DCompat::Satistics getStatistics() { return Renderer2DCompat::getStatistics(); } \
            using Satistics = Renderer2DCompat::Satistics; \
            using TextParams = Renderer2DCompat::TextParams; \
        }; \
    }
