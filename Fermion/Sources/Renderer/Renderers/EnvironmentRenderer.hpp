#pragma once
#include <cstdint>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "Renderer/Framebuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/VertexArray.hpp"

namespace Fermion
{
    class EnvironmentRenderer
    {
    public:
        struct IBLSettings
        {
            bool useIBL = true;
            uint32_t irradianceMapSize = 32;
            uint32_t prefilterMapSize = 128;
            uint32_t brdfLUTSize = 512;
            uint32_t prefilterMaxMipLevels = 5;
        };

        EnvironmentRenderer();

        void loadHDR(const std::string &hdrPath,
                     const std::shared_ptr<Framebuffer> &targetFramebuffer,
                     uint32_t viewportWidth,
                     uint32_t viewportHeight);

        void ensureIBLInitialized(const IBLSettings &settings,
                                  const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                  uint32_t viewportWidth,
                                  uint32_t viewportHeight,
                                  uint32_t *iblDrawCalls);

        void bindIBL(const std::shared_ptr<Shader> &shader, const IBLSettings &settings) const;

        void addSkyboxPass(RenderGraphLegacy &renderGraph,
                           const glm::mat4 &view,
                           const glm::mat4 &projection,
                           uint32_t *skyboxDrawCalls,
                           ResourceHandle dependency = {}) const;

        TextureCube *getEnvironmentCubemap() const;
        bool hasEnvironment() const;

        void setEnvironmentCubemap(std::unique_ptr<TextureCube> cubemap);

    private:
        void convertEquirectangularToCubemap(const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                             uint32_t viewportWidth,
                                             uint32_t viewportHeight);

        void generateIrradianceMap(const IBLSettings &settings,
                                   const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                   uint32_t viewportWidth,
                                   uint32_t viewportHeight,
                                   uint32_t *iblDrawCalls);

        void generatePrefilterMap(const IBLSettings &settings,
                                  const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                  uint32_t viewportWidth,
                                  uint32_t viewportHeight,
                                  uint32_t *iblDrawCalls);

        void generateBRDFLUT(const IBLSettings &settings,
                             const std::shared_ptr<Framebuffer> &targetFramebuffer,
                             uint32_t viewportWidth,
                             uint32_t viewportHeight,
                             uint32_t *iblDrawCalls);

        void restoreViewport(const std::shared_ptr<Framebuffer> &targetFramebuffer,
                             uint32_t viewportWidth,
                             uint32_t viewportHeight) const;

    private:
        std::unique_ptr<Texture2D> m_hdrEnvironment = nullptr;
        std::unique_ptr<TextureCube> m_environmentCubemap = nullptr;

        std::shared_ptr<VertexArray> m_cubeVA = nullptr;
        std::shared_ptr<VertexArray> m_quadVA = nullptr;

        std::shared_ptr<Pipeline> m_skyboxPipeline;
        std::shared_ptr<Pipeline> m_iblIrradiancePipeline;
        std::shared_ptr<Pipeline> m_iblPrefilterPipeline;
        std::shared_ptr<Pipeline> m_iblBrdfPipeline;
        std::shared_ptr<Pipeline> m_equirectToCubePipeline;

        std::unique_ptr<TextureCube> m_irradianceMap = nullptr;
        std::unique_ptr<TextureCube> m_prefilterMap = nullptr;
        std::unique_ptr<Texture2D> m_brdfLUT = nullptr;

        bool m_environmentLoaded = false;
        bool m_iblInitialized = false;
    };
} // namespace Fermion
