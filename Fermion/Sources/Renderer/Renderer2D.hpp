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

        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size, const glm::vec4 &color);
        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
        static void drawQuad(const glm::vec2 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D>&texture);
        static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const std::shared_ptr<Texture2D>&texture);
    };
}