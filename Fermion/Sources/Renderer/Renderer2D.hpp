#pragma once
#include "Renderer/OrthographicCameraController.hpp"
#include "Renderer/Texture.hpp"
namespace Fermion
{
    class Renderer2D
    {
    public:
        static void init();
        static void shutdown();

        static void beginScene(const OrthographicCamera &camera);
        static void endScene();

        static void flush();

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);
        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f));
        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f));

        static void drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians, const glm::vec4 &color);
        static void drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians, const glm::vec4 &color);
        static void drawRotatedQuad(const glm::vec2 &position, const glm::vec2 &size, float radians, const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f));
        static void drawRotatedQuad(const glm::vec3 &position, const glm::vec2 &size, float radians, const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f, glm::vec4 tintColor = glm::vec4(1.0f));

        struct Satistics
        {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;

            uint32_t getTotalVertexCount() { return quadCount * 4; }
            uint32_t getTotalIndexCount() { return quadCount * 6; }
        };

        static void resetStatistics();
        static Satistics getStatistics();

    private:
        static void flushAndReset();
    };
}