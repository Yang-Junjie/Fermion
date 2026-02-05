#include "ForwardRenderer.hpp"
#include "EnvironmentRenderer.hpp"
#include "ShadowMapRenderer.hpp"
#include "Renderer.hpp"
#include "Renderer/RenderCommands.hpp"
#include "Renderer/UniformBufferLayout.hpp"
#include "Renderer/UniformBuffer.hpp"
#include "Renderer/Pipeline.hpp"
namespace Fermion
{
    ForwardRenderer::ForwardRenderer()
    {
        // Phong Mesh Pipeline
        {
            PipelineSpecification meshSpec;
            meshSpec.shader = Renderer::getShaderLibrary()->get("Mesh");
            meshSpec.depthTest = true;
            meshSpec.depthWrite = true;
            meshSpec.depthOperator = DepthCompareOperator::Less;
            meshSpec.cull = CullMode::Back;

            m_phongPipeline = Pipeline::create(meshSpec);
        }

        // PBR Mesh Pipeline
        {
            PipelineSpecification pbrSpec;
            pbrSpec.shader = Renderer::getShaderLibrary()->get("PBRMesh");
            pbrSpec.depthTest = true;
            pbrSpec.depthWrite = true;
            pbrSpec.depthOperator = DepthCompareOperator::Less;
            pbrSpec.cull = CullMode::Back;

            m_pbrPipeline = Pipeline::create(pbrSpec);
        }

        // Skinned PBR Mesh Pipeline
        {
            PipelineSpecification skinnedPbrSpec;
            skinnedPbrSpec.shader = Renderer::getShaderLibrary()->get("SkinnedPBRMesh");
            skinnedPbrSpec.depthTest = true;
            skinnedPbrSpec.depthWrite = true;
            skinnedPbrSpec.depthOperator = DepthCompareOperator::Less;
            skinnedPbrSpec.cull = CullMode::Back;

            m_skinnedPBRPipeline = Pipeline::create(skinnedPbrSpec);
        }
    }

    void ForwardRenderer::addPass(RenderGraphLegacy& renderGraph,
                                   const RenderContext& context,
                                   const std::vector<MeshDrawCommand>& drawList,
                                   const ShadowMapRenderer* shadowRenderer,
                                   EnvironmentRenderer* envRenderer,
                                   ResourceHandle shadowMap,
                                   ResourceHandle sceneDepth,
                                   ResourceHandle lightingResult,
                                   bool transparentOnly,
                                   uint32_t* geometryDrawCalls,
                                   uint32_t* iblDrawCalls)
    {
        LegacyRenderGraphPass pass;
        pass.Name = transparentOnly ? "TransparentPass" : "ForwardPass";
        pass.Inputs = {shadowMap};
        if (transparentOnly)
        {
            pass.Inputs.push_back(sceneDepth);
            pass.Inputs.push_back(lightingResult);
        }
        else
        {
            pass.Outputs = {sceneDepth, lightingResult};
        }
        pass.Execute = [this, &context, &drawList, shadowRenderer, envRenderer, transparentOnly, geometryDrawCalls, iblDrawCalls](RenderCommandQueue& queue)
        {
            std::shared_ptr<Pipeline> currentPipeline = nullptr;
            EnvironmentRenderer::IBLSettings iblSettings = {
                .useIBL = context.useIBL,
                .irradianceMapSize = context.irradianceMapSize,
                .prefilterMapSize = context.prefilterMapSize,
                .brdfLUTSize = context.brdfLUTSize,
                .prefilterMaxMipLevels = context.prefilterMaxMipLevels
            };

            // Update light uniform buffer once for all forward pass draws
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
            queue.submit(CmdCustom{[lightUBO = context.lightUBO, lightData]() {
                lightUBO->setData(&lightData, sizeof(LightData));
            }});

            for (const auto& cmd : drawList)
            {
                if (!cmd.visible)
                    continue;
                if (cmd.transparent != transparentOnly)
                    continue;
                if (currentPipeline != cmd.pipeline)
                {
                    currentPipeline = cmd.pipeline;
                    queue.submit(CmdCustom{[pipeline = currentPipeline]() {
                        pipeline->bind();
                    }});
                }

                auto shader = currentPipeline->getShader();

                // Update model uniform buffer for this draw call
                ModelData modelData;
                modelData.model = cmd.transform;
                modelData.normalMatrix = glm::transpose(glm::inverse(cmd.transform));
                modelData.objectID = cmd.objectID;
                queue.submit(CmdCustom{[modelUBO = context.modelUBO, modelData]() {
                    modelUBO->setData(&modelData, sizeof(ModelData));
                }});

                // Upload bone matrices for skinned meshes
                if (cmd.isSkinned && cmd.boneMatrices && !cmd.boneMatrices->empty())
                {
                    queue.submit(CmdCustom{[boneUBO = context.boneUBO, boneMatrices = cmd.boneMatrices]() {
                        boneUBO->setData(boneMatrices->data(),
                            static_cast<uint32_t>(boneMatrices->size() * sizeof(glm::mat4)));
                    }});
                }

                if (cmd.pipeline == m_pbrPipeline || cmd.pipeline == m_skinnedPBRPipeline)
                {
                    if (envRenderer)
                    {
                        queue.submit(CmdCustom{[envRenderer, iblSettings, targetFB = context.targetFramebuffer,
                                                vpW = context.viewportWidth, vpH = context.viewportHeight,
                                                iblDrawCalls, shader]() {
                            envRenderer->ensureIBLInitialized(iblSettings, targetFB, vpW, vpH, iblDrawCalls);
                            envRenderer->bindIBL(shader, iblSettings);
                        }});
                    }
                    else
                    {
                        queue.submit(CmdCustom{[shader]() {
                            shader->setBool("u_UseIBL", false);
                        }});
                    }
                }

                // Shadow mapping - bind shadow map texture
                bool enableShadows = context.enableShadows && shadowRenderer && shadowRenderer->getShadowMapFramebuffer();
                if (enableShadows)
                {
                    queue.submit(CmdCustom{[shader, shadowFB = shadowRenderer->getShadowMapFramebuffer()]() {
                        shader->setInt("u_ShadowMap", 10);
                        shadowFB->bindDepthAttachment(10);
                    }});
                }

                // Additional directional lights (excluding the main one)
                uint32_t maxDirLights = 4;
                uint32_t dirLightCount = 0;
                if (context.environmentLight.directionalLights.size() > 1)
                {
                    dirLightCount = std::min(maxDirLights, (uint32_t)(context.environmentLight.directionalLights.size() - 1));
                }

                // Capture light data for the CmdCustom lambda
                {
                    auto envLight = context.environmentLight;
                    float normalStrength = context.normalMapStrength;
                    float toksvigStrength = context.toksvigStrength;
                    auto material = cmd.material;

                    queue.submit(CmdCustom{[shader, dirLightCount, envLight, normalStrength, toksvigStrength, material]() {
                        shader->setInt("u_DirLightCount", dirLightCount);
                        for (uint32_t i = 0; i < dirLightCount; i++)
                        {
                            const auto& l = envLight.directionalLights[i + 1]; // Skip main light at index 0
                            std::string base = "u_DirLights[" + std::to_string(i) + "]";
                            shader->setFloat3(base + ".direction", l.direction);
                            shader->setFloat3(base + ".color", l.color);
                            shader->setFloat(base + ".intensity", l.intensity);
                        }

                        // Point lights
                        uint32_t maxLights = 16;
                        uint32_t pointCount = std::min(maxLights, (uint32_t)envLight.pointLights.size());
                        shader->setInt("u_PointLightCount", pointCount);
                        for (uint32_t i = 0; i < pointCount; i++)
                        {
                            const auto& l = envLight.pointLights[i];
                            std::string base = "u_PointLights[" + std::to_string(i) + "]";
                            shader->setFloat3(base + ".position", l.position);
                            shader->setFloat3(base + ".color", l.color);
                            shader->setFloat(base + ".intensity", l.intensity);
                            shader->setFloat(base + ".range", l.range);
                        }

                        // Spot lights
                        uint32_t spotCount = std::min(maxLights, (uint32_t)envLight.spotLights.size());
                        shader->setInt("u_SpotLightCount", spotCount);
                        for (uint32_t i = 0; i < spotCount; i++)
                        {
                            const auto& l = envLight.spotLights[i];
                            std::string base = "u_SpotLights[" + std::to_string(i) + "]";
                            shader->setFloat3(base + ".position", l.position);
                            shader->setFloat3(base + ".direction", glm::normalize(l.direction));
                            shader->setFloat3(base + ".color", l.color);
                            shader->setFloat(base + ".intensity", l.intensity);
                            shader->setFloat(base + ".range", l.range);
                            shader->setFloat(base + ".innerConeAngle", l.innerConeAngle);
                            shader->setFloat(base + ".outerConeAngle", l.outerConeAngle);
                        }

                        // Normal map strength
                        shader->setFloat("u_NormalStrength", normalStrength);
                        shader->setFloat("u_ToksvigStrength", toksvigStrength);

                        if (material)
                            material->bind(shader);
                    }});
                }

                queue.submit(CmdDrawIndexed{cmd.vao, cmd.indexCount, cmd.indexOffset});
                if (geometryDrawCalls)
                    (*geometryDrawCalls)++;
            }
        };
        renderGraph.addPass(pass);
    }

} // namespace Fermion
