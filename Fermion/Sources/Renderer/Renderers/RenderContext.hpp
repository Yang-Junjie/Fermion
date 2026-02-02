#pragma once
#include "Renderer/Camera/Camera.hpp"
#include "Scene/Scene.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace Fermion
{
    class Framebuffer;
    class UniformBuffer;

    struct SceneRendererCamera
    {
        Camera camera;
        glm::mat4 view;
        float farClip = 0.0f;
        float nearClip = 0.1f;
    };

    struct RenderContext
    {
        // Shared UniformBuffers
        std::shared_ptr<UniformBuffer> cameraUBO;
        std::shared_ptr<UniformBuffer> modelUBO;
        std::shared_ptr<UniformBuffer> lightUBO;

        // Scene data
        SceneRendererCamera camera;
        EnvironmentLight environmentLight;

        // Viewport information
        uint32_t viewportWidth = 0;
        uint32_t viewportHeight = 0;

        // Target Framebuffer
        std::shared_ptr<Framebuffer> targetFramebuffer;

        // Scene settings
        float ambientIntensity = 0.1f;
        bool enableShadows = true;
        float shadowBias = 0.01f;
        float shadowSoftness = 1.0f;
        float normalMapStrength = 1.0f;
        float toksvigStrength = 1.0f;

        // IBL settings
        bool useIBL = true;
        uint32_t irradianceMapSize = 32;
        uint32_t prefilterMapSize = 128;
        uint32_t brdfLUTSize = 512;
        uint32_t prefilterMaxMipLevels = 5;
    };

} // namespace Fermion
