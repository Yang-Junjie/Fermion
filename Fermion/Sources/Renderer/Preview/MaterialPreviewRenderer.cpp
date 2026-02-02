#include "MaterialPreviewRenderer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/Model/Material.hpp"
#include "Renderer/Model/Mesh.hpp"

#include "Renderer/Texture/Texture.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace Fermion
{

    MaterialPreviewRenderer::MaterialPreviewRenderer()
    {
        initialize();
    }

    std::shared_ptr<Mesh> MaterialPreviewRenderer::createSphereMesh(
        float radius,
        uint32_t latitudeSegments,
        uint32_t longitudeSegments)
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t y = 0; y <= latitudeSegments; y++)
        {
            float v = static_cast<float>(y) / static_cast<float>(latitudeSegments);
            float theta = v * glm::pi<float>();

            float sinTheta = sin(theta);
            float cosTheta = cos(theta);

            for (uint32_t x = 0; x <= longitudeSegments; x++)
            {
                float u = static_cast<float>(x) / static_cast<float>(longitudeSegments);
                float phi = u * glm::two_pi<float>();

                float sinPhi = sin(phi);
                float cosPhi = cos(phi);

                glm::vec3 normal = {
                    cosPhi * sinTheta,
                    cosTheta,
                    sinPhi * sinTheta};

                glm::vec3 position = normal * radius;

                vertices.push_back({position,
                                    glm::normalize(normal),
                                    {1, 1, 1, 1},
                                    {u, v}});
            }
        }

        uint32_t stride = longitudeSegments + 1;

        for (uint32_t y = 0; y < latitudeSegments; y++)
        {
            for (uint32_t x = 0; x < longitudeSegments; x++)
            {
                uint32_t i0 = y * stride + x;
                uint32_t i1 = i0 + stride;
                uint32_t i2 = i1 + 1;
                uint32_t i3 = i0 + 1;

                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i2);
                indices.push_back(i0);
                indices.push_back(i3);
            }
        }

        SubMesh sub;
        sub.MaterialSlotIndex = 0;
        sub.IndexOffset = 0;
        sub.IndexCount = static_cast<uint32_t>(indices.size());

        return std::make_shared<Mesh>(
            std::move(vertices),
            std::move(indices),
            std::vector<SubMesh>{sub});
    }

    void MaterialPreviewRenderer::initialize()
    {
        if (m_initialized)
            return;

        // Load the preview shader and create pipeline
        auto previewShader = Shader::create("../Boson/Resources/Shaders/MaterialPreview.glsl");
        if (!previewShader)
        {
            Log::Error("MaterialPreviewRenderer: Failed to load MaterialPreview shader");
            return;
        }
        Log::Info("MaterialPreviewRenderer: Shader loaded: " + previewShader->getName());

        // Create pipeline with depth test and backface culling
        PipelineSpecification pipelineSpec;
        pipelineSpec.shader = previewShader;
        pipelineSpec.depthTest = true;
        pipelineSpec.depthWrite = true;
        pipelineSpec.depthOperator = DepthCompareOperator::Less;
        pipelineSpec.cull = CullMode::Back;

        m_previewPipeline = Pipeline::create(pipelineSpec);
        if (!m_previewPipeline)
        {
            Log::Error("MaterialPreviewRenderer: Failed to create preview pipeline");
            return;
        }

        // Create sphere mesh directly (16x32 segments for good quality)
        m_sphereMesh = createSphereMesh(0.5f, 16, 32);
        if (!m_sphereMesh)
        {
            Log::Error("MaterialPreviewRenderer: Failed to create sphere mesh");
            return;
        }

        Log::Info("MaterialPreviewRenderer: Initialized successfully");
        m_initialized = true;
    }

    void MaterialPreviewRenderer::setupFramebuffer(uint32_t width, uint32_t height)
    {
        if (m_framebuffer)
        {
            auto &spec = m_framebuffer->getSpecification();
            if (spec.width == width && spec.height == height)
                return;
        }

        FramebufferSpecification spec;
        spec.width = width;
        spec.height = height;
        spec.attachments = {
            FramebufferTextureFormat::RGBA8,
            FramebufferTextureFormat::Depth};
        spec.samples = 4; // 4x MSAA

        m_framebuffer = Framebuffer::create(spec);
    }

    std::unique_ptr<Texture2D> MaterialPreviewRenderer::renderPreview(
        const std::shared_ptr<Material> &material,
        const PreviewSettings &settings)
    {
        if (!m_initialized)
        {
            Log::Warn("MaterialPreviewRenderer: Not initialized, attempting to initialize now");
            initialize();
            if (!m_initialized)
            {
                Log::Error("MaterialPreviewRenderer: Failed to initialize");
                return nullptr;
            }
        }

        if (!material)
        {
            Log::Warn("MaterialPreviewRenderer: Material is null");
            return nullptr;
        }

        // Setup framebuffer with the requested size
        setupFramebuffer(settings.width, settings.height);
        if (!m_framebuffer)
        {
            Log::Error("MaterialPreviewRenderer: Failed to create framebuffer");
            return nullptr;
        }

        auto vertexArray = m_sphereMesh->getVertexArray();
        if (!vertexArray)
        {
            Log::Warn("MaterialPreviewRenderer: VertexArray is null");
            return nullptr;
        }

        // Bind framebuffer and setup render state using RenderCommand
        m_framebuffer->bind();
        RenderCommand::setViewport(0, 0, settings.width, settings.height);
        RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::clear();

        m_previewPipeline->bind();

        auto shader = m_previewPipeline->getShader();

        // Camera
        float aspectRatio = static_cast<float>(settings.width) / static_cast<float>(settings.height);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(
            settings.cameraPosition,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 viewProjection = projection * view;

        //model matrix
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));

        shader->setMat4("u_ViewProjection", viewProjection);
        shader->setMat4("u_Model", model);
        shader->setMat4("u_NormalMatrix", normalMatrix);

        shader->setFloat3("u_CameraPosition", settings.cameraPosition);

        shader->setFloat3("u_LightDirection", glm::normalize(settings.lightDirection));
        shader->setFloat3("u_LightColor", settings.lightColor);
        shader->setFloat("u_LightIntensity", settings.lightIntensity);
        shader->setFloat("u_AmbientIntensity", settings.ambientIntensity);

        material->bind(shader);

        vertexArray->bind();
        RenderCommand::drawIndexed(vertexArray);

        m_framebuffer->resolve();

        auto resultTexture = Texture2D::create(settings.width, settings.height, false);
        resultTexture->copyFromFramebuffer(m_framebuffer, 0, 0);

        m_framebuffer->unbind();

        return resultTexture;
    }

} // namespace Fermion
