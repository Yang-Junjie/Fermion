#pragma once
#include "fmpch.hpp"

#include <glm/glm.hpp>

namespace Fermion
{
    class Framebuffer;
    class Pipeline;
    class Texture2D;
    class Material;
    class Mesh;

    class MaterialPreviewRenderer
    {
    public:
        struct PreviewSettings
        {
            uint32_t width = 256;
            uint32_t height = 256;
            glm::vec3 lightDirection = glm::vec3(-0.5f, -0.7f, -0.5f);
            glm::vec3 lightColor = glm::vec3(1.0f);
            float lightIntensity = 15.0f;
            float ambientIntensity = 0.15f;
            glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 1.5f);
        };

        MaterialPreviewRenderer();
        ~MaterialPreviewRenderer() = default;

        std::unique_ptr<Texture2D> renderPreview(
            const std::shared_ptr<Material> &material,
            const PreviewSettings &settings = PreviewSettings{});

        bool isInitialized() const { return m_initialized; }

    private:
        void initialize();
        void setupFramebuffer(uint32_t width, uint32_t height);
        std::shared_ptr<Mesh> createSphereMesh(float radius, uint32_t latitudeSegments, uint32_t longitudeSegments);

        std::shared_ptr<Framebuffer> m_framebuffer;
        std::shared_ptr<Pipeline> m_previewPipeline;
        std::shared_ptr<Mesh> m_sphereMesh;
        bool m_initialized = false;
    };

} // namespace Fermion
