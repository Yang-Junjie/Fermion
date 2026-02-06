#include "SceneRenderer.hpp"
#include "Renderer2DCompat.hpp"
#include "Renderer.hpp"

#include "Renderer/Framebuffer.hpp"
#include "Renderer/UniformBuffer.hpp"
#include "Renderer/UniformBufferLayout.hpp"
#include "EnvironmentRenderer.hpp"
#include "ShadowMapRenderer.hpp"
#include "GBufferRenderer.hpp"
#include "SSGIRenderer.hpp"
#include "GTAORenderer.hpp"
#include "DeferredLightingRenderer.hpp"
#include "ForwardRenderer.hpp"
#include "OutlineRenderer.hpp"
#include "PostProcessRenderer.hpp"
#include "ProceduralSkyGenerator.hpp"
#include "InfiniteGridRenderer.hpp"
#include "Project/Project.hpp"
#include "Asset/AssetManager/RuntimeAssetManager.hpp"
#include "Scene/Components.hpp"


namespace Fermion
{
    namespace
    {

        bool IsTransparentMaterial(const std::shared_ptr<Material> &material)
        {
            if (!material)
                return false;

            if (material->getType() == MaterialType::Phong)
                return material->getDiffuseColor().a < 1.0f;

            return false;
        }
    }

    SceneRenderer::SceneRenderer()
    {
        m_debugRenderer = std::make_shared<DebugRenderer>();

        // Initialize uniform buffers
        m_cameraUniformBuffer = UniformBuffer::create(UniformBufferBinding::Camera, CameraData::getSize());
        m_modelUniformBuffer = UniformBuffer::create(UniformBufferBinding::Model, ModelData::getSize());
        m_lightUniformBuffer = UniformBuffer::create(UniformBufferBinding::Lights, LightData::getSize());
        m_boneUniformBuffer = UniformBuffer::create(UniformBufferBinding::Bones, BoneData::getSize());

        // Initialize sub-renderers
        m_gBufferRenderer = std::make_unique<GBufferRenderer>();
        m_ssgiRenderer = std::make_unique<SSGIRenderer>();
        m_gtaoRenderer = std::make_unique<GTAORenderer>();
        m_lightingRenderer = std::make_unique<DeferredLightingRenderer>();
        m_forwardRenderer = std::make_unique<ForwardRenderer>();
        m_outlineRenderer = std::make_unique<OutlineRenderer>();
        m_postProcessRenderer = std::make_unique<PostProcessRenderer>();
        m_environmentRenderer = std::make_unique<EnvironmentRenderer>();
        m_shadowRenderer = std::make_unique<ShadowMapRenderer>();
        m_proceduralSkyGenerator = std::make_unique<ProceduralSkyGenerator>();
        m_infiniteGridRenderer = std::make_unique<InfiniteGridRenderer>();

        // Use procedural sky as default environment
        generateProceduralSky();
    }

    SceneRenderer::~SceneRenderer() = default;

    void SceneRenderer::resetStatistics()
    {
        m_renderer3DStatistics = {};
    }

    void SceneRenderer::beginScene(const Camera &camera, const glm::mat4 &transform)
    {
        beginScene({camera, glm::inverse(transform)});
    }

    void SceneRenderer::beginScene(const EditorCamera &camera)
    {
        beginScene({camera, camera.getViewMatrix(), camera.getFarCilp(), camera.getNearCilp()});
    }

    void SceneRenderer::beginScene(const SceneRendererCamera &camera)
    {
        m_sceneData.sceneCamera = camera;
        m_sceneData.sceneEnvironmentLight = m_scene->m_environmentLight;
        m_cameraFrustumPlanes = Math::ExtractFrustumPlanes(camera.camera.getProjection() * camera.view);
        m_hasCameraFrustum = true;
        Renderer2DCompat::beginScene(camera.camera, camera.view);
        updateViewState(camera);
    }

    void SceneRenderer::beginOverlay(const Camera &camera, const glm::mat4 &transform)
    {
        beginOverlay({camera, glm::inverse(transform)});
    }

    void SceneRenderer::beginOverlay(const EditorCamera &camera)
    {
        beginOverlay({camera, camera.getViewMatrix(), camera.getFarCilp(), camera.getNearCilp()});
    }

    void SceneRenderer::beginOverlay(const SceneRendererCamera &camera)
    {
        m_sceneData.sceneCamera = camera;
        m_cameraFrustumPlanes = Math::ExtractFrustumPlanes(camera.camera.getProjection() * camera.view);
        m_hasCameraFrustum = true;
        Renderer2DCompat::beginScene(camera.camera, camera.view);
        updateViewState(camera);
    }

    void SceneRenderer::updateViewState(const SceneRendererCamera &camera)
    {
        const glm::mat4 viewProjection = camera.camera.getProjection() * camera.view;
        const glm::vec3 cameraPosition = glm::vec3(glm::inverse(camera.view)[3]);

        // Update camera uniform buffer (shared across all shaders)
        CameraData cameraData;
        cameraData.viewProjection = viewProjection;
        cameraData.view = camera.view;
        cameraData.projection = camera.camera.getProjection();
        cameraData.position = cameraPosition;
        m_cameraUniformBuffer->setData(&cameraData, sizeof(CameraData));
    }

    void SceneRenderer::updateRenderContext()
    {
        m_renderContext.cameraUBO = m_cameraUniformBuffer;
        m_renderContext.modelUBO = m_modelUniformBuffer;
        m_renderContext.lightUBO = m_lightUniformBuffer;
        m_renderContext.boneUBO = m_boneUniformBuffer;
        m_renderContext.camera = m_sceneData.sceneCamera;
        m_renderContext.environmentLight = m_sceneData.sceneEnvironmentLight;
        m_renderContext.viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        m_renderContext.viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
        m_renderContext.targetFramebuffer = m_targetFramebuffer;
        m_renderContext.ambientIntensity = m_sceneData.environmentSettings.ambientIntensity;
        m_renderContext.enableShadows = m_sceneData.environmentSettings.enableShadows;
        m_renderContext.shadowBias = m_sceneData.environmentSettings.shadowBias;
        m_renderContext.shadowSoftness = m_sceneData.environmentSettings.shadowSoftness;
        m_renderContext.normalMapStrength = m_sceneData.environmentSettings.normalMapStrength;
        m_renderContext.toksvigStrength = m_sceneData.environmentSettings.toksvigStrength;
        m_renderContext.useIBL = m_sceneData.environmentSettings.useIBL;
        m_renderContext.irradianceMapSize = m_sceneData.irradianceMapSize;
        m_renderContext.prefilterMapSize = m_sceneData.prefilterMapSize;
        m_renderContext.brdfLUTSize = m_sceneData.brdfLUTSize;
        m_renderContext.prefilterMaxMipLevels = m_sceneData.prefilterMaxMipLevels;

        Log::Trace(std::format("[SceneRenderer::updateRenderContext] useIBL={}, ambientIntensity={}, showSkybox={}",
                              m_renderContext.useIBL, m_renderContext.ambientIntensity, m_sceneData.environmentSettings.showSkybox));
    }

    void SceneRenderer::endScene()
    {
        FlushDrawList();
        Renderer2DCompat::endScene();
    }

    void SceneRenderer::endOverlay()
    {
        for (auto &cmd : m_meshDrawList)
        {
            if (cmd.drawOutline && cmd.visible)
                Renderer2DCompat::drawAABB(cmd.aabb, cmd.transform, m_sceneData.meshOutlineColor, cmd.objectID);
        }
        m_meshDrawList.clear();

        Renderer2DCompat::endScene();
    }

    void SceneRenderer::drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID)
    {
        if (static_cast<uint64_t>(sprite.textureHandle) != 0)
        {
            auto texture = Project::getRuntimeAssetManager()->getAsset<Texture2D>(sprite.textureHandle);
            if (texture)
            {
                Renderer2DCompat::drawQuad(transform, texture, sprite.tilingFactor, sprite.color, objectID);
            }
        }
        else
        {
            Renderer2DCompat::drawQuad(transform, sprite.color, objectID);
        }
    }

    void SceneRenderer::drawString(const std::string &string, const glm::mat4 &transform,
                                   const TextComponent &component, int objectID)
    {
        Renderer2DCompat::drawString(string, component.fontAsset, transform,
                               {component.color, component.kerning, component.lineSpacing}, objectID);
    }

    void SceneRenderer::drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade,
                                   int objectID)
    {
        Renderer2DCompat::drawCircle(transform, color, thickness, fade, objectID);
    }

    void SceneRenderer::drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color,
                                 int objectId)
    {
        Renderer2DCompat::drawRect(position, size, color, objectId);
    }

    void SceneRenderer::drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId)
    {
        Renderer2DCompat::drawRect(transform, color, objectId);
    }

    void SceneRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size, const glm::vec4 &color,
                                          int objectId)
    {
        Renderer2DCompat::drawQuadBillboard(translation, size, color, objectId);
    }
    void SceneRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                                          const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                          const glm::vec4 &tintColor, int objectId)
    {
        Renderer2DCompat::drawQuadBillboard(translation, size, texture, tilingFactor, tintColor, objectId);
    }

    void SceneRenderer::submitMesh(MeshComponent &meshComponent, glm::mat4 transform, int objectId, bool drawOutline)
    {
        if (static_cast<uint64_t>(meshComponent.meshHandle) != 0)
        {
            auto assetManager = Project::getRuntimeAssetManager();
            auto mesh = assetManager->getAsset<Mesh>(meshComponent.meshHandle);
            if (mesh)
            {
                auto vao = mesh->getVertexArray();
                const auto &submeshes = mesh->getSubMeshes();
                bool visible = true;
                if (m_hasCameraFrustum)
                {
                    const AABB worldAabb = AABB::TransformAABB(mesh->getBoundingBox(), transform);
                    visible = Math::IsAABBInsideFrustum(m_cameraFrustumPlanes, worldAabb);
                }
                for (size_t i = 0; i < submeshes.size(); i++)
                {
                    const auto &submesh = submeshes[i];
                    std::shared_ptr<Material> material;

                    AssetHandle submeshMaterialHandle = meshComponent.getSubmeshMaterial(static_cast<uint32_t>(i));
                    if (static_cast<uint64_t>(submeshMaterialHandle) != 0)
                    {
                        material = assetManager->getAsset<Material>(submeshMaterialHandle);
                    }

                    if (!material)
                    {
                        material = std::make_shared<Material>();
                        material->setMaterialType(MaterialType::PBR);
                        material->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
                        material->setMetallic(0.0f);
                        material->setRoughness(1.0f);
                        material->setAO(1.0f);
                    }

                    // Create draw command using forward renderer pipelines
                    MaterialType matType = material->getType();
                    MeshDrawCommand cmd;
                    cmd.pipeline = (matType == MaterialType::PBR) ? m_forwardRenderer->getPBRPipeline() : m_forwardRenderer->getPhongPipeline();
                    cmd.vao = vao;
                    cmd.material = material;
                    cmd.transform = transform;
                    cmd.indexCount = submesh.IndexCount;
                    cmd.indexOffset = submesh.IndexOffset;
                    cmd.objectID = objectId;
                    cmd.drawOutline = drawOutline;
                    cmd.visible = visible;
                    cmd.transparent = IsTransparentMaterial(material);
                    cmd.aabb = mesh->getBoundingBox();

                    m_meshDrawList.emplace_back(std::move(cmd));
                }
            }
        }
    }

    void SceneRenderer::submitSkinnedMesh(MeshComponent &meshComponent, AnimatorComponent &animator, glm::mat4 transform, int objectId, bool drawOutline)
    {
        if (static_cast<uint64_t>(meshComponent.meshHandle) == 0)
            return;

        auto assetManager = Project::getRuntimeAssetManager();
        auto mesh = assetManager->getAsset<Mesh>(meshComponent.meshHandle);
        if (!mesh)
            return;

        // If mesh is not skinned, fall back to regular mesh rendering
        if (!mesh->isSkinned())
        {
            submitMesh(meshComponent, transform, objectId, drawOutline);
            return;
        }

        // Load skeleton from AnimatorComponent if not already loaded
        if (!animator.runtimeSkeleton && static_cast<uint64_t>(animator.skeletonHandle) != 0)
        {
            animator.runtimeSkeleton = assetManager->getAsset<Skeleton>(animator.skeletonHandle);
        }

        // Fall back to mesh's embedded skeleton if AnimatorComponent doesn't have one
        std::shared_ptr<Skeleton> skeleton = animator.runtimeSkeleton;
        if (!skeleton)
        {
            skeleton = mesh->getSkeleton();
        }

        // If still no skeleton, fall back to regular rendering
        if (!skeleton)
        {
            submitMesh(meshComponent, transform, objectId, drawOutline);
            return;
        }

        // Load animation clips from AnimatorComponent if not already loaded
        if (animator.runtimeClips.size() != animator.animationClipHandles.size())
        {
            animator.runtimeClips.clear();
            for (const auto &clipHandle : animator.animationClipHandles)
            {
                if (static_cast<uint64_t>(clipHandle) != 0)
                {
                    auto clip = assetManager->getAsset<AnimationClip>(clipHandle);
                    animator.runtimeClips.push_back(clip);
                }
                else
                {
                    animator.runtimeClips.push_back(nullptr);
                }
            }
        }

        // Get animation clips - prefer AnimatorComponent's clips, fall back to mesh's embedded animations
        const std::vector<std::shared_ptr<AnimationClip>>* animationClips = &animator.runtimeClips;
        if (animator.runtimeClips.empty())
        {
            animationClips = &mesh->getAnimations();
        }

        // Initialize runtime animator if needed
        if (!animator.runtimeAnimator)
        {
            animator.runtimeAnimator = std::make_shared<Animator>();
            animator.runtimeAnimator->setSkeleton(skeleton);
            animator.runtimeAnimator->setSpeed(animator.speed);
            animator.runtimeAnimator->setLooping(animator.looping);

            // Play first animation clip if available
            if (!animationClips->empty() && animator.activeClipIndex < animationClips->size())
            {
                animator.runtimeAnimator->play((*animationClips)[animator.activeClipIndex]);
            }
        }

        // Update animator state from component
        animator.runtimeAnimator->setSpeed(animator.speed);
        animator.runtimeAnimator->setLooping(animator.looping);
        if (animator.playing && !animator.runtimeAnimator->isPlaying())
        {
            if (!animationClips->empty() && animator.activeClipIndex < animationClips->size())
            {
                animator.runtimeAnimator->play((*animationClips)[animator.activeClipIndex]);
            }
        }
        else if (!animator.playing && animator.runtimeAnimator->isPlaying())
        {
            animator.runtimeAnimator->pause();
        }

        auto vao = mesh->getVertexArray();
        const auto &submeshes = mesh->getSubMeshes();
        bool visible = true;
        if (m_hasCameraFrustum)
        {
            const AABB worldAabb = AABB::TransformAABB(mesh->getBoundingBox(), transform);
            visible = Math::IsAABBInsideFrustum(m_cameraFrustumPlanes, worldAabb);
        }

        const std::vector<glm::mat4>* boneMatrices = &animator.runtimeAnimator->getFinalBoneMatrices();

        for (size_t i = 0; i < submeshes.size(); i++)
        {
            const auto &submesh = submeshes[i];
            std::shared_ptr<Material> material;

            AssetHandle submeshMaterialHandle = meshComponent.getSubmeshMaterial(static_cast<uint32_t>(i));
            if (static_cast<uint64_t>(submeshMaterialHandle) != 0)
            {
                material = assetManager->getAsset<Material>(submeshMaterialHandle);
            }

            if (!material)
            {
                material = std::make_shared<Material>();
                material->setMaterialType(MaterialType::PBR);
                material->setAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
                material->setMetallic(0.0f);
                material->setRoughness(1.0f);
                material->setAO(1.0f);
            }

            // Create draw command using skinned pipelines
            MeshDrawCommand cmd;
            cmd.pipeline = m_forwardRenderer->getSkinnedPBRPipeline();
            cmd.vao = vao;
            cmd.material = material;
            cmd.transform = transform;
            cmd.indexCount = submesh.IndexCount;
            cmd.indexOffset = submesh.IndexOffset;
            cmd.objectID = objectId;
            cmd.drawOutline = drawOutline;
            cmd.visible = visible;
            cmd.transparent = IsTransparentMaterial(material);
            cmd.aabb = mesh->getBoundingBox();
            cmd.isSkinned = true;
            cmd.boneMatrices = boneMatrices;

            m_meshDrawList.emplace_back(std::move(cmd));
        }
    }

    void SceneRenderer::drawInfiniteLine(const glm::vec3 &point, const glm::vec3 &direction, const glm::vec4 &color)
    {
        float big = m_sceneData.sceneCamera.farClip * 2.0f;

        glm::vec3 p0 = point - direction * big;
        glm::vec3 p1 = point + direction * big;
        Renderer2DCompat::drawLine(p0, p1, color);
    }
    void SceneRenderer::drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color)
    {
        Renderer2DCompat::drawLine(start, end, color);
    }

    void SceneRenderer::setLineWidth(float thickness)
    {
        Renderer2DCompat::setLineWidth(thickness);
    }

    SceneRenderer::RenderStatistics SceneRenderer::getStatistics() const
    {
        Renderer2DCompat::Satistics stats2D = Renderer2DCompat::getStatistics();
        RenderStatistics result;
        result.renderer2D.drawCalls = stats2D.drawCalls;
        result.renderer2D.quadCount = stats2D.quadCount;
        result.renderer2D.lineCount = stats2D.lineCount;
        result.renderer2D.circleCount = stats2D.circleCount;
        result.renderer3D = m_renderer3DStatistics;
        return result;
    }
    void SceneRenderer::FlushDrawList()
    {
        m_renderGraph.reset();
        updateRenderContext();

        m_renderer3DStatistics.meshCount += static_cast<uint32_t>(m_meshDrawList.size());

        const FrameFlags flags = PrepareFrameFlags();
        const FrameResources resources = PrepareResources(flags);
        PrepareEnvironmentAndShadows(resources);

        if (flags.useDeferred)
            RenderDeferredPath(resources, flags);
        else
            RenderForwardPath(resources, flags);

        AddPostProcessingPasses(resources, flags);


        m_renderGraph.execute(m_commandQueue, Renderer::getRendererAPI());
        m_meshDrawList.clear();
        m_outlineIDs.clear();
    }

    void SceneRenderer::setOutlineIDs(const std::vector<int> &ids)
    {
        m_outlineIDs = ids;
    }

    void SceneRenderer::SkyboxPass(ResourceHandle lightingResult)
    {
        if (!m_environmentRenderer)
        {
            Log::Warn("[SceneRenderer::SkyboxPass] No environment renderer available");
            return;
        }

        Log::Trace(std::format("[SceneRenderer::SkyboxPass] hasEnvironment={}",
                              m_environmentRenderer->hasEnvironment()));

        m_environmentRenderer->addSkyboxPass(m_renderGraph,
                                             m_sceneData.sceneCamera.view,
                                             m_sceneData.sceneCamera.camera.getProjection(),
                                             &m_renderer3DStatistics.skyboxDrawCalls,
                                             lightingResult);
    }

    void SceneRenderer::ShadowPass(ResourceHandle shadowMap)
    {
        if (!m_shadowRenderer)
            return;

        // Use the main directional light (first one) for shadow mapping
        if (m_sceneData.sceneEnvironmentLight.directionalLights.empty())
            return;

        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
        m_shadowRenderer->addPass(
            m_renderGraph,
            shadowMap,
            m_meshDrawList,
            m_sceneData.sceneEnvironmentLight.directionalLights[0],
            m_sceneData.environmentSettings.shadowMapSize,
            m_targetFramebuffer,
            viewportWidth,
            viewportHeight,
            &m_renderer3DStatistics.shadowDrawCalls,
            m_modelUniformBuffer,
            m_lightUniformBuffer,
            m_boneUniformBuffer);
    }

    SceneRenderer::FrameFlags SceneRenderer::PrepareFrameFlags() const
    {
        FrameFlags flags;
        flags.useDeferred = m_sceneData.renderMode == RenderMode::DeferredHybrid;
        flags.useSSGI = flags.useDeferred && (m_sceneData.enableSSGI || m_sceneData.gbufferDebug == GBufferDebugMode::SSGI);
        flags.useGTAO = flags.useDeferred && (m_sceneData.enableGTAO || m_sceneData.gbufferDebug == GBufferDebugMode::GTAO);
        flags.showGBufferDebug = flags.useDeferred && (m_sceneData.gbufferDebug != GBufferDebugMode::None);

        bool hasTransparent = false;
        for (const auto &cmd : m_meshDrawList)
        {
            if (cmd.transparent)
            {
                hasTransparent = true;
                break;
            }
        }
        flags.hasTransparent = hasTransparent;

        return flags;
    }

    SceneRenderer::FrameResources SceneRenderer::PrepareResources(const FrameFlags &flags)
    {
        FrameResources resources;

        if (m_sceneData.environmentSettings.enableShadows)
            resources.shadowMap = m_renderGraph.createResource();

        resources.gBuffer = m_renderGraph.createResource();
        resources.lightingResult = m_renderGraph.createResource();
        resources.sceneDepth = m_renderGraph.createResource();

        const uint32_t viewportWidth = m_renderContext.viewportWidth;
        const uint32_t viewportHeight = m_renderContext.viewportHeight;

        if (flags.useDeferred)
            m_gBufferRenderer->ensureFramebuffer(viewportWidth, viewportHeight);

        if (flags.useSSGI)
        {
            m_ssgiRenderer->ensureFramebuffers(viewportWidth, viewportHeight);
            resources.ssgi = m_renderGraph.createResource();
        }
        else
        {
            m_ssgiRenderer->setEnabled(false);
            m_ssgiRenderer->resetAccumulation();
        }

        if (flags.useGTAO)
        {
            m_gtaoRenderer->ensureFramebuffer(viewportWidth, viewportHeight);
            resources.gtao = m_renderGraph.createResource();
        }

        return resources;
    }

    void SceneRenderer::PrepareEnvironmentAndShadows(const FrameResources &resources)
    {
        if (m_sceneData.environmentSettings.enableShadows)
            ShadowPass(resources.shadowMap);
    }

    void SceneRenderer::RenderDeferredPath(const FrameResources &resources, const FrameFlags &flags)
    {
        Log::Trace(std::format("[SceneRenderer::RenderDeferredPath] Starting deferred rendering path"));
        Log::Trace(std::format("  showSkybox: {}", m_sceneData.environmentSettings.showSkybox));
        Log::Trace(std::format("  useIBL: {}", m_sceneData.environmentSettings.useIBL));
        Log::Trace(std::format("  ambientIntensity: {}", m_sceneData.environmentSettings.ambientIntensity));

        // GBuffer Pass
        m_gBufferRenderer->addPass(
            m_renderGraph,
            m_renderContext,
            m_meshDrawList,
            m_forwardRenderer->getPBRPipeline(),
            m_environmentRenderer.get(),
            resources.gBuffer,
            resources.sceneDepth,
            &m_renderer3DStatistics.geometryDrawCalls,
            &m_renderer3DStatistics.iblDrawCalls);

        // SSGI Pass
        if (flags.useSSGI)
        {
            SSGIRenderer::Settings ssgiSettings;
            ssgiSettings.intensity = m_sceneData.ssgiIntensity;
            ssgiSettings.radius = m_sceneData.ssgiRadius;
            ssgiSettings.bias = m_sceneData.ssgiBias;
            ssgiSettings.sampleCount = m_sceneData.ssgiSampleCount;
            m_ssgiRenderer->addPass(m_renderGraph, m_renderContext, *m_gBufferRenderer, resources.ssgi, ssgiSettings);
        }

        // GTAO Pass
        if (flags.useGTAO)
        {
            GTAORenderer::Settings gtaoSettings;
            gtaoSettings.intensity = m_sceneData.gtaoIntensity;
            gtaoSettings.radius = m_sceneData.gtaoRadius;
            gtaoSettings.bias = m_sceneData.gtaoBias;
            gtaoSettings.power = m_sceneData.gtaoPower;
            gtaoSettings.sliceCount = m_sceneData.gtaoSliceCount;
            gtaoSettings.stepCount = m_sceneData.gtaoStepCount;
            m_gtaoRenderer->addPass(m_renderGraph, m_renderContext, *m_gBufferRenderer, resources.gtao, gtaoSettings);
        }

        if (flags.showGBufferDebug)
        {
            // GBuffer Debug Pass
            m_postProcessRenderer->addGBufferDebugPass(
                m_renderGraph,
                m_renderContext,
                *m_gBufferRenderer,
                m_ssgiRenderer.get(),
                m_gtaoRenderer.get(),
                static_cast<PostProcessRenderer::GBufferDebugMode>(m_sceneData.gbufferDebug),
                m_sceneData.depthViewPower,
                resources.gBuffer,
                resources.sceneDepth,
                resources.ssgi,
                resources.gtao);
        }
        else
        {
            // Deferred Lighting Pass
            m_lightingRenderer->addPass(
                m_renderGraph,
                m_renderContext,
                *m_gBufferRenderer,
                m_shadowRenderer.get(),
                m_ssgiRenderer.get(),
                m_gtaoRenderer.get(),
                m_environmentRenderer.get(),
                resources.lightingResult,
                m_sceneData.enableSSGI,
                m_sceneData.enableGTAO);

            if (m_sceneData.environmentSettings.showSkybox)
            {
                Log::Trace("[SceneRenderer] Adding skybox pass (deferred path)");
                SkyboxPass(resources.lightingResult);
            }
            else
            {
                Log::Trace("[SceneRenderer] Skybox disabled, skipping skybox pass");
            }

            if (flags.hasTransparent)
            {
                m_forwardRenderer->addPass(
                    m_renderGraph,
                    m_renderContext,
                    m_meshDrawList,
                    m_shadowRenderer.get(),
                    m_environmentRenderer.get(),
                    resources.shadowMap,
                    resources.sceneDepth,
                    resources.lightingResult,
                    true, // transparentOnly
                    &m_renderer3DStatistics.geometryDrawCalls,
                    &m_renderer3DStatistics.iblDrawCalls);
            }
        }
    }

    void SceneRenderer::RenderForwardPath(const FrameResources &resources, const FrameFlags &flags)
    {
        // Forward Pass
        m_forwardRenderer->addPass(
            m_renderGraph,
            m_renderContext,
            m_meshDrawList,
            m_shadowRenderer.get(),
            m_environmentRenderer.get(),
            resources.shadowMap,
            resources.sceneDepth,
            resources.lightingResult,
            false, // transparentOnly
            &m_renderer3DStatistics.geometryDrawCalls,
            &m_renderer3DStatistics.iblDrawCalls);

        if (m_sceneData.environmentSettings.showSkybox)
            SkyboxPass(resources.lightingResult);

        if (flags.hasTransparent)
        {
            m_forwardRenderer->addPass(
                m_renderGraph,
                m_renderContext,
                m_meshDrawList,
                m_shadowRenderer.get(),
                m_environmentRenderer.get(),
                resources.shadowMap,
                resources.sceneDepth,
                resources.lightingResult,
                true, // transparentOnly
                &m_renderer3DStatistics.geometryDrawCalls,
                &m_renderer3DStatistics.iblDrawCalls);
        }
    }

    void SceneRenderer::AddPostProcessingPasses(const FrameResources &resources, const FrameFlags &flags)
    {
        // Infinite Grid Pass
        if (m_sceneData.showInfiniteGrid && m_infiniteGridRenderer && !getScene()->isRunning())
        {
            InfiniteGridRenderer::Settings gridSettings;
            gridSettings.enabled = m_sceneData.showInfiniteGrid;
            gridSettings.plane = static_cast<GridPlane>(m_sceneData.gridPlane);
            gridSettings.gridScale = m_sceneData.gridScale;
            gridSettings.fadeDistance = m_sceneData.gridFadeDistance;
            gridSettings.gridColorThin = m_sceneData.gridColorThin;
            gridSettings.gridColorThick = m_sceneData.gridColorThick;
            gridSettings.axisColorX = m_sceneData.gridAxisColorX;
            gridSettings.axisColorZ = m_sceneData.gridAxisColorZ;

            m_infiniteGridRenderer->addPass(
                m_renderGraph,
                m_renderContext,
                gridSettings,
                resources.lightingResult,
                resources.sceneDepth);
        }

        // Depth View Pass
        if (m_sceneData.enableDepthView && !flags.showGBufferDebug)
        {
            m_postProcessRenderer->addDepthViewPass(
                m_renderGraph,
                m_renderContext,
                m_gBufferRenderer.get(),
                flags.useDeferred,
                m_sceneData.depthViewPower,
                resources.sceneDepth,
                resources.lightingResult);
        }

        OutlineRenderer::Settings outlineSettings;
        outlineSettings.color = m_sceneData.meshOutlineColor;
        outlineSettings.lineWidth = m_sceneData.outlineThickness;
        m_outlineRenderer->addPass(
            m_renderGraph,
            m_renderContext,
            flags.useDeferred ? m_gBufferRenderer.get() : nullptr,
            m_meshDrawList,
            m_outlineIDs,
            outlineSettings,
            resources.gBuffer,
            resources.sceneDepth,
            resources.lightingResult);
    }

    void SceneRenderer::loadHDREnvironment(const std::string &hdrPath)
    {
        if (!m_environmentRenderer)
            return;

        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
        m_environmentRenderer->loadHDR(hdrPath, m_targetFramebuffer, viewportWidth, viewportHeight);
    }

    void SceneRenderer::generateProceduralSky()
    {
        if (!m_environmentRenderer || !m_proceduralSkyGenerator)
            return;

        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;

        auto cubemap = m_proceduralSkyGenerator->generate(
            m_sceneData.skySettings,
            m_targetFramebuffer,
            viewportWidth,
            viewportHeight);

        m_environmentRenderer->setEnvironmentCubemap(std::move(cubemap));
    }

    std::shared_ptr<Framebuffer> SceneRenderer::getGBufferFramebuffer() const
    {
        return m_gBufferRenderer ? m_gBufferRenderer->getFramebuffer() : nullptr;
    }

    uint32_t SceneRenderer::getGBufferAttachmentRendererID(GBufferAttachment attachment) const
    {
        if (!m_gBufferRenderer)
            return 0;
        return m_gBufferRenderer->getAttachmentRendererID(static_cast<GBufferRenderer::Attachment>(attachment));
    }

    void SceneRenderer::bindGBufferAttachment(GBufferAttachment attachment, uint32_t slot) const
    {
        if (m_gBufferRenderer)
            m_gBufferRenderer->bindAttachment(static_cast<GBufferRenderer::Attachment>(attachment), slot);
    }

} // namespace Fermion
