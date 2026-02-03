#include "fmpch.hpp"
#include "ProceduralSkyGenerator.hpp"

#include "Core/Log.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/VertexArray.hpp"

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
    }

    ProceduralSkyGenerator::ProceduralSkyGenerator()
    {
        PipelineSpecification pipelineSpec;
        pipelineSpec.shader = Renderer::getShaderLibrary()->get("ProceduralSky");
        pipelineSpec.depthTest = false;
        pipelineSpec.depthWrite = false;
        pipelineSpec.cull = CullMode::None;
        m_pipeline = Pipeline::create(pipelineSpec);

        createCubeVA();
    }

    void ProceduralSkyGenerator::createCubeVA()
    {
        m_cubeVA = CreateCubeVA();
    }

    std::unique_ptr<TextureCube> ProceduralSkyGenerator::generate(
        const SkySettings& settings,
        const std::shared_ptr<Framebuffer>& targetFramebuffer,
        uint32_t viewportWidth,
        uint32_t viewportHeight)
    {
        Log::Info(std::format("Generating procedural sky cubemap (size: {}x{})...",
                              settings.cubemapSize, settings.cubemapSize));

        TextureCubeSpecification cubemapSpec;
        cubemapSpec.width = settings.cubemapSize;
        cubemapSpec.height = settings.cubemapSize;
        cubemapSpec.format = ImageFormat::RGB16F;
        cubemapSpec.generateMips = true;
        cubemapSpec.maxMipLevels = 5;
        auto cubemap = TextureCube::create(cubemapSpec);

        FramebufferSpecification fbSpec;
        fbSpec.width = settings.cubemapSize;
        fbSpec.height = settings.cubemapSize;
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
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))   
        };

        m_pipeline->bind();
        auto shader = m_pipeline->getShader();

        glm::vec3 normalizedSunDir = glm::normalize(settings.sunDirection);
        shader->setFloat3("u_SunDirection", normalizedSunDir);
        shader->setFloat("u_SunIntensity", settings.sunIntensity);
        shader->setFloat("u_SunAngularRadius", settings.sunAngularRadius);
        shader->setFloat3("u_SkyColorZenith", settings.skyColorZenith);
        shader->setFloat3("u_SkyColorHorizon", settings.skyColorHorizon);
        shader->setFloat3("u_GroundColor", settings.groundColor);
        shader->setFloat("u_SkyExposure", settings.skyExposure);
        shader->setMat4("u_Projection", captureProjection);

        for (uint32_t i = 0; i < 6; ++i)
        {
            shader->setMat4("u_View", captureViews[i]);

            captureFB->bind();
            Renderer::getRendererAPI().setViewport(0, 0, settings.cubemapSize, settings.cubemapSize);
            Renderer::getRendererAPI().clear();
            Renderer::getRendererAPI().drawIndexed(m_cubeVA, m_cubeVA->getIndexBuffer()->getCount());

            cubemap->copyFromFramebuffer(captureFB, i, 0);
        }

        captureFB->unbind();

        cubemap->generateMipmaps();

        if (targetFramebuffer)
        {
            targetFramebuffer->bind();
        }
        else if (viewportWidth > 0 && viewportHeight > 0)
        {
            Renderer::getRendererAPI().setViewport(0, 0, viewportWidth, viewportHeight);
        }

        Log::Info("Procedural sky cubemap generation completed");

        return cubemap;
    }

} // namespace Fermion
