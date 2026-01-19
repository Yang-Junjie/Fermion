#include "fmpch.hpp"
#include "EnvironmentRenderer.hpp"

#include "Core/Log.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "Renderer/Renderers/Renderer3D.hpp"

#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace Fermion
{
    namespace
    {
        std::shared_ptr<VertexArray> CreateCubeVA()
        {
            float skyboxVertices[] = {
                -1.0f, 1.0f, -1.0f,
                -1.0f, -1.0f, -1.0f,
                1.0f, -1.0f, -1.0f,
                1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f,
                1.0f, -1.0f, 1.0f,
                1.0f, 1.0f, 1.0f};

            uint32_t skyboxIndices[] = {
                0, 1, 2, 2, 3, 0,
                4, 5, 6, 6, 7, 4,
                4, 5, 1, 1, 0, 4,
                3, 2, 6, 6, 7, 3,
                4, 0, 3, 3, 7, 4,
                1, 5, 6, 6, 2, 1};

            auto vertexBuffer = VertexBuffer::create(skyboxVertices, sizeof(skyboxVertices));
            vertexBuffer->setLayout({{ShaderDataType::Float3, "a_Position"}});

            auto indexBuffer = IndexBuffer::create(skyboxIndices, sizeof(skyboxIndices) / sizeof(uint32_t));

            auto cubeVA = VertexArray::create();
            cubeVA->addVertexBuffer(vertexBuffer);
            cubeVA->setIndexBuffer(indexBuffer);
            return cubeVA;
        }

        std::shared_ptr<VertexArray> CreateFullscreenQuad()
        {
            float quadVertices[] = {
                // positions        // texture coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

            uint32_t quadIndices[] = {0, 1, 2, 2, 3, 0};

            auto quadVB = VertexBuffer::create(quadVertices, sizeof(quadVertices));
            quadVB->setLayout({{ShaderDataType::Float3, "a_Position"},
                               {ShaderDataType::Float2, "a_TexCoords"}});

            auto quadIB = IndexBuffer::create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));

            auto quadVA = VertexArray::create();
            quadVA->addVertexBuffer(quadVB);
            quadVA->setIndexBuffer(quadIB);
            return quadVA;
        }

        uint32_t ClampViewport(uint32_t value)
        {
            return value == 0 ? 1 : value;
        }
    }

    EnvironmentRenderer::EnvironmentRenderer()
    {
        {
            PipelineSpecification skyboxSpec;
            skyboxSpec.shader = Renderer::getShaderLibrary()->get("Skybox");
            skyboxSpec.depthTest = true;
            skyboxSpec.depthWrite = false;
            skyboxSpec.depthOperator = DepthCompareOperator::LessOrEqual;
            skyboxSpec.cull = CullMode::None;
            m_skyboxPipeline = Pipeline::create(skyboxSpec);
        }

        {
            PipelineSpecification iblIrradianceSpec;
            iblIrradianceSpec.shader = Renderer::getShaderLibrary()->get("IBLPreprocess");
            iblIrradianceSpec.depthTest = false;
            iblIrradianceSpec.depthWrite = false;
            iblIrradianceSpec.cull = CullMode::None;
            m_iblIrradiancePipeline = Pipeline::create(iblIrradianceSpec);

            PipelineSpecification iblPrefilterSpec;
            iblPrefilterSpec.shader = Renderer::getShaderLibrary()->get("IBLPrefilter");
            iblPrefilterSpec.depthTest = false;
            iblPrefilterSpec.depthWrite = false;
            iblPrefilterSpec.cull = CullMode::None;
            m_iblPrefilterPipeline = Pipeline::create(iblPrefilterSpec);

            PipelineSpecification iblBRDFSpec;
            iblBRDFSpec.shader = Renderer::getShaderLibrary()->get("BRDFLUT");
            iblBRDFSpec.depthTest = false;
            iblBRDFSpec.depthWrite = false;
            iblBRDFSpec.cull = CullMode::None;
            m_iblBrdfPipeline = Pipeline::create(iblBRDFSpec);

            PipelineSpecification equirectToCubeSpec;
            equirectToCubeSpec.shader = Renderer::getShaderLibrary()->get("EquirectToCube");
            equirectToCubeSpec.depthTest = false;
            equirectToCubeSpec.depthWrite = false;
            equirectToCubeSpec.cull = CullMode::None;
            m_equirectToCubePipeline = Pipeline::create(equirectToCubeSpec);
        }

        m_cubeVA = CreateCubeVA();
        m_quadVA = CreateFullscreenQuad();
    }

    void EnvironmentRenderer::loadHDR(const std::string &hdrPath,
                                      const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                      uint32_t viewportWidth,
                                      uint32_t viewportHeight)
    {
        Log::Info(std::format("Loading HDR environment: {}", hdrPath));

        m_hdrEnvironment = Texture2D::create(hdrPath);
        if (!m_hdrEnvironment || !m_hdrEnvironment->isLoaded())
        {
            Log::Error(std::format("Failed to load HDR environment from: {}", hdrPath));
            return;
        }

        Log::Info(std::format("HDR loaded: {}x{}", m_hdrEnvironment->getWidth(), m_hdrEnvironment->getHeight()));

        convertEquirectangularToCubemap(targetFramebuffer, viewportWidth, viewportHeight);

        m_environmentLoaded = true;
        m_iblInitialized = false;

        Log::Info("HDR environment loaded successfully");
    }

    void EnvironmentRenderer::ensureIBLInitialized(const IBLSettings &settings,
                                                   const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                                   uint32_t viewportWidth,
                                                   uint32_t viewportHeight,
                                                   uint32_t *iblDrawCalls)
    {
        if (m_iblInitialized || !m_environmentCubemap || !settings.useIBL)
            return;

        Log::Info("Initializing IBL from HDR environment...");

        generateIrradianceMap(settings, targetFramebuffer, viewportWidth, viewportHeight, iblDrawCalls);
        generatePrefilterMap(settings, targetFramebuffer, viewportWidth, viewportHeight, iblDrawCalls);
        generateBRDFLUT(settings, targetFramebuffer, viewportWidth, viewportHeight, iblDrawCalls);

        m_iblInitialized = true;
        Log::Info("IBL initialization completed");
    }

    void EnvironmentRenderer::bindIBL(const std::shared_ptr<Shader> &shader, const IBLSettings &settings) const
    {
        const bool enableIBL = settings.useIBL && m_iblInitialized;
        shader->setBool("u_UseIBL", enableIBL);
        if (!enableIBL)
            return;

        shader->setInt("u_IrradianceMap", 11);
        shader->setInt("u_PrefilterMap", 12);
        shader->setInt("u_BRDFLT", 13);
        shader->setFloat("u_PrefilterMaxLOD", static_cast<float>(settings.prefilterMaxMipLevels - 1));

        if (m_irradianceMap)
            m_irradianceMap->bind(11);
        if (m_prefilterMap)
            m_prefilterMap->bind(12);
        if (m_brdfLUT)
            m_brdfLUT->bind(13);
    }

    void EnvironmentRenderer::addSkyboxPass(RenderGraph &renderGraph,
                                            const glm::mat4 &view,
                                            const glm::mat4 &projection,
                                            uint32_t *skyboxDrawCalls,
                                            ResourceHandle dependency) const
    {
        RenderGraphPass pass;
        pass.Name = "SkyboxPass";
        if (dependency.isValid())
            pass.Inputs = {dependency};
        pass.Execute = [this, view, projection, skyboxDrawCalls](CommandBuffer &commandBuffer)
        {
            if (!m_environmentCubemap)
                return;

            SkyboxDrawCommand cmd;
            cmd.pipeline = m_skyboxPipeline;
            cmd.vao = m_cubeVA;
            cmd.cubemap = m_environmentCubemap.get();
            cmd.view = view;
            cmd.projection = projection;

            if (cmd.pipeline && cmd.vao && cmd.cubemap && skyboxDrawCalls)
                (*skyboxDrawCalls)++;

            Renderer3D::recordSkyboxPass(commandBuffer, cmd);
        };

        renderGraph.addPass(pass);
    }

    TextureCube *EnvironmentRenderer::getEnvironmentCubemap() const
    {
        return m_environmentCubemap.get();
    }

    bool EnvironmentRenderer::hasEnvironment() const
    {
        return m_environmentLoaded && m_environmentCubemap != nullptr;
    }

    void EnvironmentRenderer::convertEquirectangularToCubemap(const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                                              uint32_t viewportWidth,
                                                              uint32_t viewportHeight)
    {
        if (!m_hdrEnvironment)
        {
            Log::Error("No HDR environment loaded for conversion");
            return;
        }

        Log::Info("Converting equirectangular HDR to cubemap...");

        uint32_t cubemapSize = 4096;
        TextureCubeSpecification cubemapSpec;
        cubemapSpec.width = cubemapSize;
        cubemapSpec.height = cubemapSize;
        cubemapSpec.format = ImageFormat::RGB16F;
        cubemapSpec.generateMips = true;
        cubemapSpec.maxMipLevels = 5;
        m_environmentCubemap = TextureCube::create(cubemapSpec);

        FramebufferSpecification fbSpec;
        fbSpec.width = cubemapSize;
        fbSpec.height = cubemapSize;
        fbSpec.attachments = {FramebufferTextureFormat::RGB16F};
        fbSpec.swapChainTarget = false;
        auto captureFB = Framebuffer::create(fbSpec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        m_equirectToCubePipeline->bind();
        auto shader = m_equirectToCubePipeline->getShader();
        shader->setInt("u_EquirectangularMap", 0);
        shader->setMat4("u_Projection", captureProjection);
        m_hdrEnvironment->bind(0);

        for (uint32_t i = 0; i < 6; ++i)
        {
            shader->setMat4("u_View", captureViews[i]);
            captureFB->bind();
            RenderCommand::setViewport(0, 0, cubemapSize, cubemapSize);
            RenderCommand::clear();
            RenderCommand::drawIndexed(m_cubeVA, m_cubeVA->getIndexBuffer()->getCount());

            m_environmentCubemap->copyFromFramebuffer(captureFB, i, 0);
        }

        captureFB->unbind();
        m_environmentCubemap->generateMipmaps();

        restoreViewport(targetFramebuffer, viewportWidth, viewportHeight);

        Log::Info("Equirectangular to cubemap conversion completed");
    }

    void EnvironmentRenderer::generateIrradianceMap(const IBLSettings &settings,
                                                    const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                                    uint32_t viewportWidth,
                                                    uint32_t viewportHeight,
                                                    uint32_t *iblDrawCalls)
    {
        Log::Info(std::format("Generating irradiance map (size: {}x{})...",
                              settings.irradianceMapSize, settings.irradianceMapSize));

        TextureCubeSpecification irradianceSpec;
        irradianceSpec.width = settings.irradianceMapSize;
        irradianceSpec.height = settings.irradianceMapSize;
        irradianceSpec.format = ImageFormat::RGB16F;
        irradianceSpec.generateMips = false;
        irradianceSpec.maxMipLevels = 1;
        m_irradianceMap = TextureCube::create(irradianceSpec);

        FramebufferSpecification fbSpec;
        fbSpec.width = settings.irradianceMapSize;
        fbSpec.height = settings.irradianceMapSize;
        fbSpec.attachments = {FramebufferTextureFormat::RGB16F};
        fbSpec.swapChainTarget = false;
        auto captureFB = Framebuffer::create(fbSpec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        m_iblIrradiancePipeline->bind();
        auto shader = m_iblIrradiancePipeline->getShader();
        shader->setInt("u_EnvironmentMap", 0);
        shader->setMat4("u_Projection", captureProjection);
        m_environmentCubemap->bind(0);

        for (uint32_t i = 0; i < 6; ++i)
        {
            shader->setMat4("u_View", captureViews[i]);
            captureFB->bind();
            RenderCommand::setViewport(0, 0, settings.irradianceMapSize, settings.irradianceMapSize);
            RenderCommand::clear();
            RenderCommand::drawIndexed(m_cubeVA, m_cubeVA->getIndexBuffer()->getCount());
            if (iblDrawCalls)
                (*iblDrawCalls)++;

            m_irradianceMap->copyFromFramebuffer(captureFB, i, 0);
        }

        captureFB->unbind();

        restoreViewport(targetFramebuffer, viewportWidth, viewportHeight);

        Log::Info("Irradiance map generation completed");
    }

    void EnvironmentRenderer::generatePrefilterMap(const IBLSettings &settings,
                                                   const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                                   uint32_t viewportWidth,
                                                   uint32_t viewportHeight,
                                                   uint32_t *iblDrawCalls)
    {
        TextureCubeSpecification prefilterSpec;
        prefilterSpec.width = settings.prefilterMapSize;
        prefilterSpec.height = settings.prefilterMapSize;
        prefilterSpec.format = ImageFormat::RGB16F;
        prefilterSpec.generateMips = true;
        prefilterSpec.maxMipLevels = settings.prefilterMaxMipLevels;
        m_prefilterMap = TextureCube::create(prefilterSpec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        m_iblPrefilterPipeline->bind();
        auto shader = m_iblPrefilterPipeline->getShader();
        shader->setInt("u_EnvironmentMap", 0);
        shader->setMat4("u_Projection", captureProjection);
        m_environmentCubemap->bind(0);

        for (uint32_t mip = 0; mip < settings.prefilterMaxMipLevels; ++mip)
        {
            uint32_t mipWidth = static_cast<uint32_t>(settings.prefilterMapSize * std::pow(0.5, mip));
            uint32_t mipHeight = mipWidth;

            FramebufferSpecification fbSpec;
            fbSpec.width = ClampViewport(mipWidth);
            fbSpec.height = ClampViewport(mipHeight);
            fbSpec.attachments = {FramebufferTextureFormat::RGB16F};
            fbSpec.swapChainTarget = false;
            auto captureFB = Framebuffer::create(fbSpec);

            float roughness = static_cast<float>(mip) / static_cast<float>(settings.prefilterMaxMipLevels - 1);
            shader->setFloat("u_Roughness", roughness);

            for (uint32_t i = 0; i < 6; ++i)
            {
                shader->setMat4("u_View", captureViews[i]);
                captureFB->bind();
                RenderCommand::setViewport(0, 0, fbSpec.width, fbSpec.height);
                RenderCommand::clear();
                RenderCommand::drawIndexed(m_cubeVA, m_cubeVA->getIndexBuffer()->getCount());
                if (iblDrawCalls)
                    (*iblDrawCalls)++;

                m_prefilterMap->copyFromFramebuffer(captureFB, i, mip);
            }

            captureFB->unbind();
        }

        restoreViewport(targetFramebuffer, viewportWidth, viewportHeight);
    }

    void EnvironmentRenderer::generateBRDFLUT(const IBLSettings &settings,
                                              const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                              uint32_t viewportWidth,
                                              uint32_t viewportHeight,
                                              uint32_t *iblDrawCalls)
    {
        TextureSpecification brdfSpec;
        brdfSpec.Width = settings.brdfLUTSize;
        brdfSpec.Height = settings.brdfLUTSize;
        brdfSpec.Format = ImageFormat::RG16F;
        brdfSpec.GenerateMips = false;
        m_brdfLUT = Texture2D::create(brdfSpec);

        FramebufferSpecification fbSpec;
        fbSpec.width = settings.brdfLUTSize;
        fbSpec.height = settings.brdfLUTSize;
        fbSpec.attachments = {FramebufferTextureFormat::RG16F};
        fbSpec.swapChainTarget = false;
        auto captureFB = Framebuffer::create(fbSpec);

        captureFB->bind();
        RenderCommand::setViewport(0, 0, settings.brdfLUTSize, settings.brdfLUTSize);

        m_iblBrdfPipeline->bind();
        RenderCommand::clear();
        RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
        if (iblDrawCalls)
            (*iblDrawCalls)++;

        m_brdfLUT->copyFromFramebuffer(captureFB, 0, 0);
        captureFB->unbind();

        restoreViewport(targetFramebuffer, viewportWidth, viewportHeight);
    }

    void EnvironmentRenderer::restoreViewport(const std::shared_ptr<Framebuffer> &targetFramebuffer,
                                              uint32_t viewportWidth,
                                              uint32_t viewportHeight) const
    {
        if (targetFramebuffer)
        {
            targetFramebuffer->bind();
            return;
        }

        if (viewportWidth > 0 && viewportHeight > 0)
            RenderCommand::setViewport(0, 0, viewportWidth, viewportHeight);
    }
} // namespace Fermion
