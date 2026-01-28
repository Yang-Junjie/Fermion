#include "DeferredLightingRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "SSGIRenderer.hpp"
#include "GTAORenderer.hpp"
#include "EnvironmentRenderer.hpp"
#include "ShadowMapRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/UniformBufferLayout.hpp"

namespace Fermion
{
    DeferredLightingRenderer::DeferredLightingRenderer()
    {
        // Deferred Lighting Pipeline
        {
            PipelineSpecification lightingSpec;
            lightingSpec.shader = Renderer::getShaderLibrary()->get("DeferredLighting");
            lightingSpec.depthTest = false;
            lightingSpec.depthWrite = false;
            lightingSpec.cull = CullMode::None;

            m_pipeline = Pipeline::create(lightingSpec);
        }

        // Fullscreen quad
        float quadVertices[] = {
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

        uint32_t quadIndices[] = {0, 1, 2, 2, 3, 0};

        auto quadVB = VertexBuffer::create(quadVertices, sizeof(quadVertices));
        quadVB->setLayout({{ShaderDataType::Float3, "a_Position"},
                           {ShaderDataType::Float2, "a_TexCoords"}});

        auto quadIB = IndexBuffer::create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));

        m_quadVA = VertexArray::create();
        m_quadVA->addVertexBuffer(quadVB);
        m_quadVA->setIndexBuffer(quadIB);
    }

    void DeferredLightingRenderer::addPass(RenderGraphLegacy& renderGraph,
                                            const RenderContext& context,
                                            const GBufferRenderer& gBuffer,
                                            const ShadowMapRenderer* shadowRenderer,
                                            const SSGIRenderer* ssgiRenderer,
                                            const GTAORenderer* gtaoRenderer,
                                            EnvironmentRenderer* envRenderer,
                                            ResourceHandle lightingResult,
                                            bool enableSSGI,
                                            bool enableGTAO)
    {
        LegacyRenderGraphPass pass;
        pass.Name = "LightingPass";
        pass.Inputs = {};  // Dependencies are accessed via references
        pass.Outputs = {lightingResult};
        pass.Execute = [this, &context, &gBuffer, shadowRenderer, ssgiRenderer, gtaoRenderer, envRenderer, enableSSGI, enableGTAO](CommandBuffer& commandBuffer)
        {
            commandBuffer.record([this, &context, &gBuffer, shadowRenderer, ssgiRenderer, gtaoRenderer, envRenderer, enableSSGI, enableGTAO](RendererAPI& api)
            {
                auto gBufferFramebuffer = gBuffer.getFramebuffer();
                if (!gBufferFramebuffer || !m_pipeline)
                    return;

                if (context.targetFramebuffer)
                {
                    context.targetFramebuffer->bind();
                }
                else
                {
                    if (context.viewportWidth > 0 && context.viewportHeight > 0)
                        RenderCommand::setViewport(0, 0, context.viewportWidth, context.viewportHeight);
                }

                m_pipeline->bind();
                auto shader = m_pipeline->getShader();

                shader->setInt("u_GBufferAlbedo", 0);
                shader->setInt("u_GBufferNormal", 1);
                shader->setInt("u_GBufferMaterial", 2);
                shader->setInt("u_GBufferEmissive", 3);
                shader->setInt("u_GBufferDepth", 4);
                shader->setInt("u_SSGI", 5);
                shader->setInt("u_GTAO", 6);

                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Albedo), 0);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Normal), 1);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Material), 2);
                gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferRenderer::Attachment::Emissive), 3);
                gBufferFramebuffer->bindDepthAttachment(4);

                const bool useSSGI = enableSSGI && ssgiRenderer && ssgiRenderer->getResultFramebuffer();
                shader->setBool("u_EnableSSGI", useSSGI);
                if (useSSGI)
                    ssgiRenderer->getResultFramebuffer()->bindColorAttachment(0, 5);

                const bool useGTAO = enableGTAO && gtaoRenderer && gtaoRenderer->getResultFramebuffer();
                shader->setBool("u_EnableGTAO", useGTAO);
                if (useGTAO)
                    gtaoRenderer->getResultFramebuffer()->bindColorAttachment(0, 6);

                const glm::mat4 viewProjection = context.camera.camera.getProjection() * context.camera.view;
                const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
                shader->setMat4("u_InverseViewProjection", inverseViewProjection);

                // Update light uniform buffer
                LightData lightData;
                lightData.lightSpaceMatrix = shadowRenderer ? shadowRenderer->getLightSpaceMatrix() : glm::mat4(1.0f);

                // Main directional light (first one, used for shadow mapping)
                if (!context.environmentLight.directionalLights.empty())
                {
                    const auto& mainLight = context.environmentLight.directionalLights[0];
                    lightData.dirLightDirection = -mainLight.direction;
                    lightData.dirLightIntensity = mainLight.intensity;
                    lightData.dirLightColor = mainLight.color;
                }
                else
                {
                    lightData.dirLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
                    lightData.dirLightIntensity = 0.0f;
                    lightData.dirLightColor = glm::vec3(0.0f);
                }

                lightData.shadowBias = context.shadowBias;
                lightData.shadowSoftness = context.shadowSoftness;
                lightData.enableShadows = (context.enableShadows && shadowRenderer && shadowRenderer->getShadowMapFramebuffer()) ? 1 : 0;
                lightData.numDirLights = std::max(0, std::min(4, (int)context.environmentLight.directionalLights.size() - 1));
                lightData.ambientIntensity = context.ambientIntensity;
                lightData.numPointLights = std::min(16u, (uint32_t)context.environmentLight.pointLights.size());
                lightData.numSpotLights = std::min(16u, (uint32_t)context.environmentLight.spotLights.size());
                context.lightUBO->setData(&lightData, sizeof(LightData));

                EnvironmentRenderer::IBLSettings iblSettings = {
                    .useIBL = context.useIBL,
                    .irradianceMapSize = context.irradianceMapSize,
                    .prefilterMapSize = context.prefilterMapSize,
                    .brdfLUTSize = context.brdfLUTSize,
                    .prefilterMaxMipLevels = context.prefilterMaxMipLevels
                };

                if (envRenderer)
                {
                    envRenderer->bindIBL(shader, iblSettings);
                }
                else
                {
                    shader->setBool("u_UseIBL", false);
                }

                bool enableShadows = context.enableShadows && shadowRenderer && shadowRenderer->getShadowMapFramebuffer();
                if (enableShadows)
                {
                    shader->setInt("u_ShadowMap", 10);
                    shadowRenderer->getShadowMapFramebuffer()->bindDepthAttachment(10);
                }

                // Additional directional lights (excluding the main one)
                uint32_t maxDirLights = 4;
                uint32_t dirLightCount = 0;
                if (context.environmentLight.directionalLights.size() > 1)
                {
                    dirLightCount = std::min(maxDirLights, (uint32_t)(context.environmentLight.directionalLights.size() - 1));
                }
                shader->setInt("u_DirLightCount", dirLightCount);
                for (uint32_t i = 0; i < dirLightCount; i++)
                {
                    const auto& l = context.environmentLight.directionalLights[i + 1]; // Skip main light at index 0
                    std::string base = "u_DirLights[" + std::to_string(i) + "]";
                    shader->setFloat3(base + ".direction", l.direction);
                    shader->setFloat3(base + ".color", l.color);
                    shader->setFloat(base + ".intensity", l.intensity);
                }

                // Point and spot lights
                uint32_t maxLights = 16;
                uint32_t pointCount = std::min(maxLights, (uint32_t)context.environmentLight.pointLights.size());
                shader->setInt("u_PointLightCount", pointCount);
                for (uint32_t i = 0; i < pointCount; i++)
                {
                    const auto& l = context.environmentLight.pointLights[i];
                    std::string base = "u_PointLights[" + std::to_string(i) + "]";
                    shader->setFloat3(base + ".position", l.position);
                    shader->setFloat3(base + ".color", l.color);
                    shader->setFloat(base + ".intensity", l.intensity);
                    shader->setFloat(base + ".range", l.range);
                }

                uint32_t spotCount = std::min(maxLights, (uint32_t)context.environmentLight.spotLights.size());
                shader->setInt("u_SpotLightCount", spotCount);
                for (uint32_t i = 0; i < spotCount; i++)
                {
                    const auto& l = context.environmentLight.spotLights[i];
                    std::string base = "u_SpotLights[" + std::to_string(i) + "]";
                    shader->setFloat3(base + ".position", l.position);
                    shader->setFloat3(base + ".direction", glm::normalize(l.direction));
                    shader->setFloat3(base + ".color", l.color);
                    shader->setFloat(base + ".intensity", l.intensity);
                    shader->setFloat(base + ".range", l.range);
                    shader->setFloat(base + ".innerConeAngle", l.innerConeAngle);
                    shader->setFloat(base + ".outerConeAngle", l.outerConeAngle);
                }

                RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
            });
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
