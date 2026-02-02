#pragma once
#include "RenderContext.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include <array>
#include <memory>
#include <glm/glm.hpp>

namespace Fermion
{
    class GBufferRenderer;
    class Framebuffer;
    class Pipeline;
    class VertexArray;

    class SSGIRenderer
    {
    public:
        struct Settings
        {
            float intensity = 1.0f;
            float radius = 1.0f;
            float bias = 0.05f;
            int sampleCount = 16;
        };

        SSGIRenderer();

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const GBufferRenderer& gBuffer,
                     ResourceHandle ssgiOutput,
                     const Settings& settings);

        std::shared_ptr<Framebuffer> getResultFramebuffer() const { return m_currentFramebuffer; }

        void resetAccumulation();

        void ensureFramebuffers(uint32_t width, uint32_t height);

        bool isEnabled() const { return m_wasEnabled; }
        void setEnabled(bool enabled) { m_wasEnabled = enabled; }

    private:
        bool hasSettingsChanged(const Settings& settings, const glm::mat4& viewProjection) const;

        std::shared_ptr<Pipeline> m_pipeline;
        std::shared_ptr<VertexArray> m_quadVA;
        std::array<std::shared_ptr<Framebuffer>, 2> m_framebuffers;
        std::shared_ptr<Framebuffer> m_currentFramebuffer;

        // Temporal accumulation state
        uint32_t m_historyIndex = 0;
        uint32_t m_frameIndex = 0;
        bool m_historyValid = false;
        bool m_wasEnabled = false;
        glm::mat4 m_lastViewProjection = glm::mat4(1.0f);
        Settings m_lastSettings;
    };

} // namespace Fermion
