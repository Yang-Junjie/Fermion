#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

namespace Fermion
{
    class TextureCube;
    class Framebuffer;
    class Pipeline;
    class VertexArray;

    class ProceduralSkyGenerator
    {
    public:
        struct SkySettings
        {
            glm::vec3 sunDirection = glm::normalize(glm::vec3(0.4f, 0.6f, 0.3f));
            float sunIntensity = 20.0f;
            float sunAngularRadius = 0.02f;
            glm::vec3 skyColorZenith = glm::vec3(0.15f, 0.3f, 0.6f);
            glm::vec3 skyColorHorizon = glm::vec3(0.6f, 0.5f, 0.4f);
            glm::vec3 groundColor = glm::vec3(0.1f, 0.08f, 0.06f);
            float skyExposure = 1.5f;
            uint32_t cubemapSize = 512;
        };

        ProceduralSkyGenerator();
        ~ProceduralSkyGenerator() = default;

        std::unique_ptr<TextureCube> generate(
            const SkySettings& settings,
            const std::shared_ptr<Framebuffer>& targetFramebuffer,
            uint32_t viewportWidth,
            uint32_t viewportHeight);

    private:
        std::shared_ptr<Pipeline> m_pipeline;
        std::shared_ptr<VertexArray> m_cubeVA;
        void createCubeVA();
    };

} // namespace Fermion
