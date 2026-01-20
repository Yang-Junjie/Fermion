#include "SceneRenderer.hpp"
#include "Renderer2D.hpp"
#include "Renderer/RendererBackend.hpp"
#include "Renderer/RenderCommand.hpp"
#include "EnvironmentRenderer.hpp"
#include "ShadowMapRenderer.hpp"
#include "Project/Project.hpp"
#include "Renderer.hpp"
#include <cfloat>
#include <cmath>

namespace Fermion
{
    namespace
    {
        std::array<glm::vec4, 6> ExtractFrustumPlanes(const glm::mat4 &viewProjection)
        {
            glm::mat4 m = glm::transpose(viewProjection);
            std::array<glm::vec4, 6> planes = {
                m[3] + m[0],
                m[3] - m[0],
                m[3] + m[1],
                m[3] - m[1],
                m[3] + m[2],
                m[3] - m[2]};

            for (auto &plane : planes)
            {
                float length = glm::length(glm::vec3(plane));
                if (length > 0.0f)
                    plane /= length;
            }

            return planes;
        }

        AABB TransformAABB(const AABB &aabb, const glm::mat4 &transform)
        {
            glm::vec3 min(FLT_MAX);
            glm::vec3 max(-FLT_MAX);

            const glm::vec3 corners[8] = {
                {aabb.min.x, aabb.min.y, aabb.min.z},
                {aabb.max.x, aabb.min.y, aabb.min.z},
                {aabb.min.x, aabb.max.y, aabb.min.z},
                {aabb.max.x, aabb.max.y, aabb.min.z},
                {aabb.min.x, aabb.min.y, aabb.max.z},
                {aabb.max.x, aabb.min.y, aabb.max.z},
                {aabb.min.x, aabb.max.y, aabb.max.z},
                {aabb.max.x, aabb.max.y, aabb.max.z}};

            for (const auto &corner : corners)
            {
                glm::vec3 world = glm::vec3(transform * glm::vec4(corner, 1.0f));
                min = glm::min(min, world);
                max = glm::max(max, world);
            }

            return {min, max};
        }

        bool IsAABBInsideFrustum(const std::array<glm::vec4, 6> &planes, const AABB &aabb)
        {
            for (const auto &plane : planes)
            {
                const glm::vec3 normal(plane.x, plane.y, plane.z);
                const glm::vec3 positive = {
                    normal.x >= 0.0f ? aabb.max.x : aabb.min.x,
                    normal.y >= 0.0f ? aabb.max.y : aabb.min.y,
                    normal.z >= 0.0f ? aabb.max.z : aabb.min.z};

                if (glm::dot(normal, positive) + plane.w < 0.0f)
                    return false;
            }

            return true;
        }

        bool IsTransparentMaterial(const std::shared_ptr<Material> &material)
        {
            if (!material)
                return false;

            if (material->getType() == MaterialType::Phong)
                return material->getDiffuseColor().a < 1.0f;

            return false;
        }

        bool HasMatrixChanged(const glm::mat4 &a, const glm::mat4 &b, float epsilon)
        {
            for (int col = 0; col < 4; ++col)
            {
                for (int row = 0; row < 4; ++row)
                {
                    if (std::abs(a[col][row] - b[col][row]) > epsilon)
                        return true;
                }
            }
            return false;
        }

    }
    SceneRenderer::SceneRenderer()
    {
        m_debugRenderer = std::make_shared<DebugRenderer>();

        createPipelines();

        // Fullscreen quad for depth view
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

        m_depthViewQuadVA = VertexArray::create();
        m_depthViewQuadVA->addVertexBuffer(quadVB);
        m_depthViewQuadVA->setIndexBuffer(quadIB);

        loadHDREnvironment(m_sceneData.hdrPath);
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
        m_cameraFrustumPlanes = ExtractFrustumPlanes(camera.camera.getProjection() * camera.view);
        m_hasCameraFrustum = true;
        Renderer2D::beginScene(camera.camera, camera.view);
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
        m_cameraFrustumPlanes = ExtractFrustumPlanes(camera.camera.getProjection() * camera.view);
        m_hasCameraFrustum = true;
        Renderer2D::beginScene(camera.camera, camera.view);
        updateViewState(camera);
    }

    void SceneRenderer::updateViewState(const SceneRendererCamera &camera)
    {
        const glm::mat4 viewProjection = camera.camera.getProjection() * camera.view;
        const glm::vec3 cameraPosition = glm::vec3(glm::inverse(camera.view)[3]);

        if (m_MeshPipeline)
        {
            m_MeshPipeline->bind();
            auto meshShader = m_MeshPipeline->getShader();
            meshShader->setMat4("u_ViewProjection", viewProjection);
        }

        if (m_PBRMeshPipeline)
        {
            m_PBRMeshPipeline->bind();
            auto pbrShader = m_PBRMeshPipeline->getShader();
            pbrShader->setMat4("u_ViewProjection", viewProjection);
            pbrShader->setFloat3("u_CameraPosition", cameraPosition);
        }
    }

    void SceneRenderer::endScene()
    {
        FlushDrawList();
        Renderer2D::endScene();
    }

    void SceneRenderer::endOverlay()
    {
        for (auto &cmd : s_MeshDrawList)
        {
            if (cmd.drawOutline && cmd.visible)
                Renderer2D::drawAABB(cmd.aabb, cmd.transform, m_sceneData.meshOutlineColor, cmd.objectID);
        }
        s_MeshDrawList.clear();

        Renderer2D::endScene();
    }

    void SceneRenderer::drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID)
    {
        if (static_cast<uint64_t>(sprite.textureHandle) != 0)
        {
            auto texture = Project::getRuntimeAssetManager()->getAsset<Texture2D>(sprite.textureHandle);
            Renderer2D::drawQuad(transform, texture,
                                 sprite.tilingFactor, sprite.color, objectID);
        }
        else
        {
            Renderer2D::drawQuad(transform, sprite.color, objectID);
        }
    }

    void SceneRenderer::drawString(const std::string &string, const glm::mat4 &transform,
                                   const TextComponent &component, int objectID)
    {
        Renderer2D::drawString(string, component.fontAsset, transform,
                               {component.color, component.kerning, component.lineSpacing}, objectID);
    }

    void SceneRenderer::drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade,
                                   int objectID)
    {
        Renderer2D::drawCircle(transform, color, thickness, fade, objectID);
    }

    void SceneRenderer::drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color,
                                 int objectId)
    {
        Renderer2D::drawRect(position, size, color, objectId);
    }

    void SceneRenderer::drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId)
    {
        Renderer2D::drawRect(transform, color, objectId);
    }

    void SceneRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size, const glm::vec4 &color,
                                          int objectId)
    {
        Renderer2D::drawQuadBillboard(translation, size, color, objectId);
    }
    void SceneRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                                          const std::shared_ptr<Texture2D> &texture, float tilingFactor,
                                          const glm::vec4 &tintColor, int objectId)
    {
        Renderer2D::drawQuadBillboard(translation, size, texture, tilingFactor, tintColor, objectId);
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
                    const AABB worldAabb = TransformAABB(mesh->getBoundingBox(), transform);
                    visible = IsAABBInsideFrustum(m_cameraFrustumPlanes, worldAabb);
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

                    // 创建绘制命令
                    MaterialType matType = material->getType();
                    MeshDrawCommand cmd;
                    cmd.pipeline = (matType == MaterialType::PBR) ? m_PBRMeshPipeline : m_MeshPipeline;
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

                    s_MeshDrawList.emplace_back(std::move(cmd));
                }
            }
        }
    }
    void SceneRenderer::drawInfiniteLine(const glm::vec3 &point, const glm::vec3 &direction, const glm::vec4 &color)
    {
        float big = m_sceneData.sceneCamera.farClip * 2.0f;

        glm::vec3 p0 = point - direction * big;
        glm::vec3 p1 = point + direction * big;
        Renderer2D::drawLine(p0, p1, color);
    }
    void SceneRenderer::drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color)
    {
        Renderer2D::drawLine(start, end, color);
    }

    void SceneRenderer::setLineWidth(float thickness)
    {
        Renderer2D::setLineWidth(thickness);
    }

    SceneRenderer::RenderStatistics SceneRenderer::getStatistics() const
    {
        Renderer2D::Satistics stats2D = Renderer2D::getStatistics();
        RenderStatistics result;
        result.renderer2D.drawCalls = stats2D.drawCalls;
        result.renderer2D.quadCount = stats2D.quadCount;
        result.renderer2D.lineCount = stats2D.lineCount;
        result.renderer2D.circleCount = stats2D.circleCount;
        result.renderer3D = m_renderer3DStatistics;
        return result;
    }

    void SceneRenderer::setOutlineIDs(const std::vector<int> &ids)
    {
        m_outlineIDs = ids;
    }

    void SceneRenderer::createPipelines()
    {
        // Phong Mesh Pipeline
        {
            PipelineSpecification meshSpec;
            meshSpec.shader = Renderer::getShaderLibrary()->get("Mesh");
            meshSpec.depthTest = true;
            meshSpec.depthWrite = true;
            meshSpec.depthOperator = DepthCompareOperator::Less;
            meshSpec.cull = CullMode::Back;

            m_MeshPipeline = Pipeline::create(meshSpec);
        }

        // PBR Mesh Pipeline
        {
            PipelineSpecification pbrSpec;
            pbrSpec.shader = Renderer::getShaderLibrary()->get("PBRMesh");
            pbrSpec.depthTest = true;
            pbrSpec.depthWrite = true;
            pbrSpec.depthOperator = DepthCompareOperator::Less;
            pbrSpec.cull = CullMode::Back;

            m_PBRMeshPipeline = Pipeline::create(pbrSpec);
        }

        // G-Buffer Mesh Pipeline (Phong)
        {
            PipelineSpecification gbufferSpec;
            gbufferSpec.shader = Renderer::getShaderLibrary()->get("GBufferMesh");
            gbufferSpec.depthTest = true;
            gbufferSpec.depthWrite = true;
            gbufferSpec.depthOperator = DepthCompareOperator::Less;
            gbufferSpec.cull = CullMode::Back;

            m_GBufferMeshPipeline = Pipeline::create(gbufferSpec);
        }

        // G-Buffer Mesh Pipeline (PBR)
        {
            PipelineSpecification gbufferPbrSpec;
            gbufferPbrSpec.shader = Renderer::getShaderLibrary()->get("GBufferPBRMesh");
            gbufferPbrSpec.depthTest = true;
            gbufferPbrSpec.depthWrite = true;
            gbufferPbrSpec.depthOperator = DepthCompareOperator::Less;
            gbufferPbrSpec.cull = CullMode::Back;

            m_GBufferPBRMeshPipeline = Pipeline::create(gbufferPbrSpec);
        }

        // Deferred Lighting Pipeline
        {
            PipelineSpecification lightingSpec;
            lightingSpec.shader = Renderer::getShaderLibrary()->get("DeferredLighting");
            lightingSpec.depthTest = false;
            lightingSpec.depthWrite = false;
            lightingSpec.cull = CullMode::None;

            m_DeferredLightingPipeline = Pipeline::create(lightingSpec);
        }

        // G-Buffer Debug Pipeline
        {
            PipelineSpecification debugSpec;
            debugSpec.shader = Renderer::getShaderLibrary()->get("GBufferDebug");
            debugSpec.depthTest = false;
            debugSpec.depthWrite = false;
            debugSpec.cull = CullMode::None;

            m_GBufferDebugPipeline = Pipeline::create(debugSpec);
        }

        // G-Buffer Outline Pipeline
        {
            PipelineSpecification outlineSpec;
            outlineSpec.shader = Renderer::getShaderLibrary()->get("GBufferOutline");
            outlineSpec.depthTest = false;
            outlineSpec.depthWrite = false;
            outlineSpec.cull = CullMode::None;

            m_GBufferOutlinePipeline = Pipeline::create(outlineSpec);
        }

        // SSGI Pipeline
        {
            PipelineSpecification ssgiSpec;
            ssgiSpec.shader = Renderer::getShaderLibrary()->get("SSGI");
            ssgiSpec.depthTest = false;
            ssgiSpec.depthWrite = false;
            ssgiSpec.cull = CullMode::None;

            m_SSGIPipeline = Pipeline::create(ssgiSpec);
        }

        // GTAO Pipeline
        {
            PipelineSpecification gtaoSpec;
            gtaoSpec.shader = Renderer::getShaderLibrary()->get("GTAO");
            gtaoSpec.depthTest = false;
            gtaoSpec.depthWrite = false;
            gtaoSpec.cull = CullMode::None;

            m_GTAOPipeline = Pipeline::create(gtaoSpec);
        }

        m_environmentRenderer = std::make_unique<EnvironmentRenderer>();
        m_shadowRenderer = std::make_unique<ShadowMapRenderer>();

        // DepthView Pipeline
        {
            PipelineSpecification depthViewSpec;
            depthViewSpec.shader = Renderer::getShaderLibrary()->get("DepthView");
            depthViewSpec.depthTest = false;
            depthViewSpec.depthWrite = false;
            depthViewSpec.cull = CullMode::None;
            m_DepthViewPipeline = Pipeline::create(depthViewSpec);
        }
    }

    void SceneRenderer::ForwardPass(ResourceHandle shadowMap, ResourceHandle sceneDepth, ResourceHandle lightingResult)
    {
        m_RenderGraph.addPass(
            {.Name = "ForwardPass",
             .Inputs = {shadowMap},
             .Outputs = {sceneDepth, lightingResult},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordForwardPass(commandBuffer, false);
             }});
    }

    void SceneRenderer::GBufferPass(ResourceHandle gBuffer, ResourceHandle sceneDepth)
    {
        m_RenderGraph.addPass(
            {.Name = "GBufferPass",
             .Outputs = {gBuffer, sceneDepth},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordGBufferPass(commandBuffer);
             }});
    }

    void SceneRenderer::recordGBufferPass(CommandBuffer &commandBuffer)
    {
        commandBuffer.record([this](RendererAPI &api)
                             {
            if (!m_gBufferFramebuffer)
                return;

            m_gBufferFramebuffer->bind();
            RenderCommand::setBlendEnabled(false);
            RenderCommand::setClearColor({0.0f, 0.0f, 0.0f, 1.0f});
            RenderCommand::clear();
            m_gBufferFramebuffer->clearAttachment(static_cast<uint32_t>(GBufferAttachment::ObjectID), -1);

            std::shared_ptr<Pipeline> currentPipeline = nullptr;
            const glm::mat4 viewProjection = m_sceneData.sceneCamera.camera.getProjection() * m_sceneData.sceneCamera.view;
            EnvironmentRenderer::IBLSettings iblSettings = {
                .useIBL = m_sceneData.useIBL,
                .irradianceMapSize = m_sceneData.irradianceMapSize,
                .prefilterMapSize = m_sceneData.prefilterMapSize,
                .brdfLUTSize = m_sceneData.brdfLUTSize,
                .prefilterMaxMipLevels = m_sceneData.prefilterMaxMipLevels
            };
            uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
            uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;

            if (m_environmentRenderer)
            {
                m_environmentRenderer->ensureIBLInitialized(iblSettings, m_targetFramebuffer, viewportWidth, viewportHeight,
                                                            &m_renderer3DStatistics.iblDrawCalls);
            }

            for (auto &cmd : s_MeshDrawList)
            {
                if (!cmd.visible || cmd.transparent)
                    continue;

                const bool isPbr = cmd.pipeline == m_PBRMeshPipeline;
                auto desiredPipeline = isPbr ? m_GBufferPBRMeshPipeline : m_GBufferMeshPipeline;
                if (!desiredPipeline)
                    continue;

                if (currentPipeline != desiredPipeline)
                {
                    currentPipeline = desiredPipeline;
                    currentPipeline->bind();
                    auto shader = currentPipeline->getShader();
                    shader->setMat4("u_ViewProjection", viewProjection);
                    if (isPbr)
                    {
                        shader->setFloat("u_NormalStrength", m_sceneData.normalMapStrength);
                        shader->setFloat("u_ToksvigStrength", m_sceneData.toksvigStrength);
                    }
                }

                auto shader = currentPipeline->getShader();
                shader->setMat4("u_Model", cmd.transform);
                shader->setInt("u_ObjectID", cmd.objectID);

                if (cmd.material)
                    cmd.material->bind(shader);

                RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                m_renderer3DStatistics.geometryDrawCalls++;
            }

            if (m_targetFramebuffer)
            {
                Framebuffer::blit(m_gBufferFramebuffer, m_targetFramebuffer, {
                    .mask = FramebufferBlitMask::Depth
                });
                m_targetFramebuffer->bind();
            }
            else
            {
                m_gBufferFramebuffer->unbind();
                uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
                uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
                if (viewportWidth > 0 && viewportHeight > 0)
                    RenderCommand::setViewport(0, 0, viewportWidth, viewportHeight);
            RenderCommand::setBlendEnabled(true);
            } });
    }

    void SceneRenderer::LightingPass(ResourceHandle gBuffer, ResourceHandle shadowMap, ResourceHandle sceneDepth, ResourceHandle ssgi,
                                     ResourceHandle gtao, ResourceHandle lightingResult)
    {
        m_RenderGraph.addPass(
            {.Name = "LightingPass",
             .Inputs = {gBuffer, shadowMap, sceneDepth, ssgi, gtao},
             .Outputs = {lightingResult},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordLightingPass(commandBuffer);
             }});
    }

    void SceneRenderer::recordLightingPass(CommandBuffer &commandBuffer)
    {
        commandBuffer.record([this](RendererAPI &api)
                             {
        if (!m_gBufferFramebuffer || !m_DeferredLightingPipeline)
            return;

        if (m_targetFramebuffer)
        {
            m_targetFramebuffer->bind();
        }
        else
        {
            uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
            uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
            if (viewportWidth > 0 && viewportHeight > 0)
                RenderCommand::setViewport(0, 0, viewportWidth, viewportHeight);
        }

        m_DeferredLightingPipeline->bind();
        auto shader = m_DeferredLightingPipeline->getShader();

        shader->setInt("u_GBufferAlbedo", 0);
        shader->setInt("u_GBufferNormal", 1);
        shader->setInt("u_GBufferMaterial", 2);
        shader->setInt("u_GBufferEmissive", 3);
        shader->setInt("u_GBufferDepth", 4);
        shader->setInt("u_SSGI", 5);
        shader->setInt("u_GTAO", 6);

        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Albedo), 0);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Normal), 1);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Material), 2);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Emissive), 3);
        m_gBufferFramebuffer->bindDepthAttachment(4);
        const bool useSSGI = m_sceneData.enableSSGI && m_ssgiFramebuffer;
        shader->setBool("u_EnableSSGI", useSSGI);
        if (useSSGI)
            m_ssgiFramebuffer->bindColorAttachment(0, 5);
        const bool useGTAO = m_sceneData.enableGTAO && m_gtaoFramebuffer;
        shader->setBool("u_EnableGTAO", useGTAO);
        if (useGTAO)
            m_gtaoFramebuffer->bindColorAttachment(0, 6);

        const glm::mat4 viewProjection = m_sceneData.sceneCamera.camera.getProjection() * m_sceneData.sceneCamera.view;
        const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);
        shader->setMat4("u_InverseViewProjection", inverseViewProjection);

        glm::vec3 cameraPos = glm::vec3(glm::inverse(m_sceneData.sceneCamera.view)[3]);
        shader->setFloat3("u_CameraPosition", cameraPos);
        shader->setFloat("u_AmbientIntensity", m_sceneData.ambientIntensity);

        EnvironmentRenderer::IBLSettings iblSettings = {
            .useIBL = m_sceneData.useIBL,
            .irradianceMapSize = m_sceneData.irradianceMapSize,
            .prefilterMapSize = m_sceneData.prefilterMapSize,
            .brdfLUTSize = m_sceneData.brdfLUTSize,
            .prefilterMaxMipLevels = m_sceneData.prefilterMaxMipLevels
        };
        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;

        if (m_environmentRenderer)
        {
            m_environmentRenderer->ensureIBLInitialized(iblSettings, m_targetFramebuffer, viewportWidth, viewportHeight,
                                                        &m_renderer3DStatistics.iblDrawCalls);
            m_environmentRenderer->bindIBL(shader, iblSettings);
        }
        else
        {
            shader->setBool("u_UseIBL", false);
        }

        bool enableShadows = m_sceneData.enableShadows && m_shadowRenderer && m_shadowRenderer->getShadowMapFramebuffer();
        shader->setBool("u_EnableShadows", enableShadows);
        if (enableShadows)
        {
            shader->setMat4("u_LightSpaceMatrix", m_shadowRenderer->getLightSpaceMatrix());
            shader->setFloat("u_ShadowBias", m_sceneData.shadowBias);
            shader->setFloat("u_ShadowSoftness", m_sceneData.shadowSoftness);
            shader->setInt("u_ShadowMap", 10);

            m_shadowRenderer->getShadowMapFramebuffer()->bindDepthAttachment(10);
        }
        else
        {
            shader->setMat4("u_LightSpaceMatrix", glm::mat4(1.0f));
        }

        const auto &dirLight = m_sceneData.sceneEnvironmentLight.directionalLight;
        shader->setFloat3("u_DirectionalLight.direction", -dirLight.direction);
        shader->setFloat3("u_DirectionalLight.color", dirLight.color);
        shader->setFloat("u_DirectionalLight.intensity", dirLight.intensity);

        uint32_t maxLights = 16;
        uint32_t pointCount = std::min(maxLights, (uint32_t)m_sceneData.sceneEnvironmentLight.pointLights.size());
        shader->setInt("u_PointLightCount", pointCount);
        for (uint32_t i = 0; i < pointCount; i++)
        {
            const auto &l = m_sceneData.sceneEnvironmentLight.pointLights[i];
            std::string base = "u_PointLights[" + std::to_string(i) + "]";
            shader->setFloat3(base + ".position", l.position);
            shader->setFloat3(base + ".color", l.color);
            shader->setFloat(base + ".intensity", l.intensity);
            shader->setFloat(base + ".range", l.range);
        }

        uint32_t spotCount = std::min(maxLights, (uint32_t)m_sceneData.sceneEnvironmentLight.spotLights.size());
        shader->setInt("u_SpotLightCount", spotCount);
        for (uint32_t i = 0; i < spotCount; i++)
        {
            const auto &l = m_sceneData.sceneEnvironmentLight.spotLights[i];
            std::string base = "u_SpotLights[" + std::to_string(i) + "]";
            shader->setFloat3(base + ".position", l.position);
            shader->setFloat3(base + ".direction", glm::normalize(l.direction));
            shader->setFloat3(base + ".color", l.color);
            shader->setFloat(base + ".intensity", l.intensity);
            shader->setFloat(base + ".range", l.range);
            shader->setFloat(base + ".innerConeAngle", l.innerConeAngle);
            shader->setFloat(base + ".outerConeAngle", l.outerConeAngle);
        }

        RenderCommand::drawIndexed(m_depthViewQuadVA, m_depthViewQuadVA->getIndexBuffer()->getCount()); });
    }

    void SceneRenderer::SSGIPass(ResourceHandle gBuffer, ResourceHandle sceneDepth, ResourceHandle ssgi)
    {
        m_RenderGraph.addPass(
            {.Name = "SSGIPass",
             .Inputs = {gBuffer, sceneDepth},
             .Outputs = {ssgi},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordSSGIPass(commandBuffer);
             }});
    }

    void SceneRenderer::recordSSGIPass(CommandBuffer &commandBuffer)
    {
        auto gBufferFramebuffer = m_gBufferFramebuffer;
        auto ssgiPipeline = m_SSGIPipeline;
        auto depthViewQuadVA = m_depthViewQuadVA;
        if (!gBufferFramebuffer || !ssgiPipeline || !depthViewQuadVA || !m_ssgiFramebuffers[0] || !m_ssgiFramebuffers[1])
            return;

        const glm::mat4 viewProjection = m_sceneData.sceneCamera.camera.getProjection() * m_sceneData.sceneCamera.view;
        const glm::mat4 inverseViewProjection = glm::inverse(viewProjection);

        bool resetAccumulation = !m_ssgiHistoryValid || !m_ssgiWasEnabled;
        const float epsilon = 1e-4f;
        if (!resetAccumulation && HasMatrixChanged(viewProjection, m_lastSSGIViewProjection, epsilon))
            resetAccumulation = true;
        if (!resetAccumulation)
        {
            if (std::abs(m_sceneData.ssgiRadius - m_lastSSGIRadius) > epsilon ||
                std::abs(m_sceneData.ssgiBias - m_lastSSGIBias) > epsilon ||
                std::abs(m_sceneData.ssgiIntensity - m_lastSSGIIntensity) > epsilon ||
                m_sceneData.ssgiSampleCount != m_lastSSGISampleCount)
                resetAccumulation = true;
        }

        if (resetAccumulation)
            m_ssgiFrameIndex = 0;

        int sampleCount = m_sceneData.ssgiSampleCount;
        if (sampleCount < 1)
            sampleCount = 1;
        if (sampleCount > 64)
            sampleCount = 64;

        const uint32_t historyIndex = m_ssgiHistoryIndex;
        const uint32_t currentIndex = (historyIndex + 1) % 2;
        auto historyFramebuffer = m_ssgiFramebuffers[historyIndex];
        auto currentFramebuffer = m_ssgiFramebuffers[currentIndex];
        if (!historyFramebuffer || !currentFramebuffer)
            return;

        const int frameIndex = static_cast<int>(m_ssgiFrameIndex);
        const float radius = m_sceneData.ssgiRadius;
        const float bias = m_sceneData.ssgiBias;
        const float intensity = m_sceneData.ssgiIntensity;

        commandBuffer.record([gBufferFramebuffer, ssgiPipeline, depthViewQuadVA, currentFramebuffer, historyFramebuffer,
                              viewProjection, inverseViewProjection, sampleCount, radius, bias, intensity, frameIndex](RendererAPI &api)
                             {
            if (!gBufferFramebuffer || !ssgiPipeline || !currentFramebuffer || !depthViewQuadVA)
                return;

            currentFramebuffer->bind();
            RenderCommand::setBlendEnabled(false);
            RenderCommand::setClearColor({0.0f, 0.0f, 0.0f, 1.0f});
            RenderCommand::clear();

            ssgiPipeline->bind();
            auto shader = ssgiPipeline->getShader();

            shader->setInt("u_GBufferAlbedo", 0);
            shader->setInt("u_GBufferNormal", 1);
            shader->setInt("u_GBufferDepth", 2);
            shader->setInt("u_SSGIHistory", 3);

            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Albedo), 0);
            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Normal), 1);
            gBufferFramebuffer->bindDepthAttachment(2);
            historyFramebuffer->bindColorAttachment(0, 3);

            shader->setMat4("u_ViewProjection", viewProjection);
            shader->setMat4("u_InverseViewProjection", inverseViewProjection);

            shader->setInt("u_SSGISampleCount", sampleCount);
            shader->setFloat("u_SSGIRadius", radius);
            shader->setFloat("u_SSGIBias", bias);
            shader->setFloat("u_SSGIIntensity", intensity);
            shader->setInt("u_FrameIndex", frameIndex);

            RenderCommand::drawIndexed(depthViewQuadVA, depthViewQuadVA->getIndexBuffer()->getCount());
            RenderCommand::setBlendEnabled(true); });

        m_ssgiFramebuffer = currentFramebuffer;
        m_ssgiHistoryIndex = currentIndex;
        m_ssgiHistoryValid = true;
        m_ssgiWasEnabled = true;
        m_lastSSGIViewProjection = viewProjection;
        m_lastSSGIRadius = radius;
        m_lastSSGIBias = bias;
        m_lastSSGIIntensity = intensity;
        m_lastSSGISampleCount = sampleCount;

        m_ssgiFrameIndex++;
    }

    void SceneRenderer::GTAOPass(ResourceHandle gBuffer, ResourceHandle sceneDepth, ResourceHandle gtao)
    {
        m_RenderGraph.addPass(
            {.Name = "GTAOPass",
             .Inputs = {gBuffer, sceneDepth},
             .Outputs = {gtao},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordGTAOPass(commandBuffer);
             }});
    }

    void SceneRenderer::recordGTAOPass(CommandBuffer &commandBuffer)
    {
        auto gBufferFramebuffer = m_gBufferFramebuffer;
        auto gtaoPipeline = m_GTAOPipeline;
        auto depthViewQuadVA = m_depthViewQuadVA;
        auto gtaoFramebuffer = m_gtaoFramebuffer;
        if (!gBufferFramebuffer || !gtaoPipeline || !depthViewQuadVA || !gtaoFramebuffer)
            return;

        const glm::mat4 projection = m_sceneData.sceneCamera.camera.getProjection();
        const glm::mat4 inverseProjection = glm::inverse(projection);
        const glm::mat4 view = m_sceneData.sceneCamera.view;

        int sliceCount = m_sceneData.gtaoSliceCount;
        if (sliceCount < 1)
            sliceCount = 1;
        if (sliceCount > 12)
            sliceCount = 12;

        int stepCount = m_sceneData.gtaoStepCount;
        if (stepCount < 1)
            stepCount = 1;
        if (stepCount > 16)
            stepCount = 16;

        const float radius = m_sceneData.gtaoRadius;
        const float bias = m_sceneData.gtaoBias;
        const float power = m_sceneData.gtaoPower;
        const float intensity = m_sceneData.gtaoIntensity;

        commandBuffer.record([gBufferFramebuffer, gtaoPipeline, depthViewQuadVA, gtaoFramebuffer,
                              projection, inverseProjection, view, sliceCount, stepCount, radius, bias, power, intensity](RendererAPI &api)
                             {
            if (!gBufferFramebuffer || !gtaoPipeline || !gtaoFramebuffer || !depthViewQuadVA)
                return;

            gtaoFramebuffer->bind();
            RenderCommand::setBlendEnabled(false);
            RenderCommand::setClearColor({1.0f, 1.0f, 1.0f, 1.0f});
            RenderCommand::clear();

            gtaoPipeline->bind();
            auto shader = gtaoPipeline->getShader();

            shader->setInt("u_GBufferNormal", 0);
            shader->setInt("u_GBufferDepth", 1);

            gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Normal), 0);
            gBufferFramebuffer->bindDepthAttachment(1);

            shader->setMat4("u_Projection", projection);
            shader->setMat4("u_InverseProjection", inverseProjection);
            shader->setMat4("u_View", view);
            shader->setFloat("u_GTAORadius", radius);
            shader->setFloat("u_GTAOBias", bias);
            shader->setFloat("u_GTAOPower", power);
            shader->setFloat("u_GTAOIntensity", intensity);
            shader->setInt("u_GTAOSliceCount", sliceCount);
            shader->setInt("u_GTAOStepCount", stepCount);

            RenderCommand::drawIndexed(depthViewQuadVA, depthViewQuadVA->getIndexBuffer()->getCount());
            RenderCommand::setBlendEnabled(true); });
    }

    void SceneRenderer::GBufferDebugPass(ResourceHandle gBuffer, ResourceHandle sceneDepth, ResourceHandle ssgi, ResourceHandle gtao)
    {
        m_RenderGraph.addPass(
            {.Name = "GBufferDebugPass",
             .Inputs = {gBuffer, sceneDepth, ssgi, gtao},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordGBufferDebugPass(commandBuffer);
             }});
    }

    void SceneRenderer::recordGBufferDebugPass(CommandBuffer &commandBuffer)
    {
        commandBuffer.record([this](RendererAPI &api)
                             {
        if (!m_gBufferFramebuffer || !m_GBufferDebugPipeline)
            return;

        if (m_targetFramebuffer)
        {
            m_targetFramebuffer->bind();
        }
        else
        {
            uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
            uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
            if (viewportWidth > 0 && viewportHeight > 0)
                RenderCommand::setViewport(0, 0, viewportWidth, viewportHeight);
        }

        m_GBufferDebugPipeline->bind();
        auto shader = m_GBufferDebugPipeline->getShader();

        shader->setInt("u_GBufferAlbedo", 0);
        shader->setInt("u_GBufferNormal", 1);
        shader->setInt("u_GBufferMaterial", 2);
        shader->setInt("u_GBufferEmissive", 3);
        shader->setInt("u_GBufferObjectID", 4);
        shader->setInt("u_GBufferDepth", 5);
        shader->setInt("u_SSGI", 6);
        shader->setInt("u_GTAO", 7);
        shader->setInt("u_Mode", static_cast<int>(m_sceneData.gbufferDebug));
        shader->setFloat("u_Near", m_sceneData.sceneCamera.nearClip);
        shader->setFloat("u_Far", m_sceneData.sceneCamera.farClip);
        shader->setFloat("u_DepthPower", m_sceneData.depthViewPower);

        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Albedo), 0);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Normal), 1);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Material), 2);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Emissive), 3);
        m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::ObjectID), 4);
        m_gBufferFramebuffer->bindDepthAttachment(5);
        if (m_ssgiFramebuffer)
            m_ssgiFramebuffer->bindColorAttachment(0, 6);
        if (m_gtaoFramebuffer)
            m_gtaoFramebuffer->bindColorAttachment(0, 7);

        RenderCommand::drawIndexed(m_depthViewQuadVA, m_depthViewQuadVA->getIndexBuffer()->getCount()); });
    }

    void SceneRenderer::TransparentPass(ResourceHandle shadowMap, ResourceHandle sceneDepth, ResourceHandle lightingResult)
    {
        m_RenderGraph.addPass(
            {.Name = "TransparentPass",
             .Inputs = {shadowMap, sceneDepth, lightingResult},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 recordForwardPass(commandBuffer, true);
             }});
    }

    void SceneRenderer::recordForwardPass(CommandBuffer &commandBuffer, bool drawTransparent)
    {
        commandBuffer.record([this, drawTransparent](RendererAPI &api)
                             {
        std::shared_ptr<Pipeline> currentPipeline = nullptr;
        EnvironmentRenderer::IBLSettings iblSettings = {
            .useIBL = m_sceneData.useIBL,
            .irradianceMapSize = m_sceneData.irradianceMapSize,
            .prefilterMapSize = m_sceneData.prefilterMapSize,
            .brdfLUTSize = m_sceneData.brdfLUTSize,
            .prefilterMaxMipLevels = m_sceneData.prefilterMaxMipLevels
        };
        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;

        for (auto &cmd : s_MeshDrawList)
        {
            if (!cmd.visible)
                continue;
            if (cmd.transparent != drawTransparent)
                continue;
            if (currentPipeline != cmd.pipeline)
            {
                currentPipeline = cmd.pipeline;
                currentPipeline->bind();
            }

            auto shader = currentPipeline->getShader();
            shader->setMat4("u_Model", cmd.transform);
            shader->setInt("u_ObjectID", cmd.objectID);

            if (cmd.pipeline == m_PBRMeshPipeline)
            {
                glm::vec3 cameraPos = glm::vec3(glm::inverse(m_sceneData.sceneCamera.view)[3]);
                shader->setFloat3("u_CameraPosition", cameraPos);
                shader->setFloat("u_AmbientIntensity", m_sceneData.ambientIntensity);

                if (m_environmentRenderer)
                {
                    m_environmentRenderer->ensureIBLInitialized(iblSettings, m_targetFramebuffer, viewportWidth, viewportHeight,
                                                                &m_renderer3DStatistics.iblDrawCalls);
                    m_environmentRenderer->bindIBL(shader, iblSettings);
                }
                else
                {
                    shader->setBool("u_UseIBL", false);
                }
            }

            // Shadow mapping
            bool enableShadows = m_sceneData.enableShadows && m_shadowRenderer && m_shadowRenderer->getShadowMapFramebuffer();
            shader->setBool("u_EnableShadows", enableShadows);
            if (enableShadows)
            {
                shader->setMat4("u_LightSpaceMatrix", m_shadowRenderer->getLightSpaceMatrix());
                shader->setFloat("u_ShadowBias", m_sceneData.shadowBias);
                shader->setFloat("u_ShadowSoftness", m_sceneData.shadowSoftness);
                shader->setInt("u_ShadowMap", 10);

                m_shadowRenderer->getShadowMapFramebuffer()->bindDepthAttachment(10);
            }

            // Directional light
            const auto &dirLight = m_sceneData.sceneEnvironmentLight.directionalLight;
            shader->setFloat3("u_DirectionalLight.direction", -dirLight.direction);
            shader->setFloat3("u_DirectionalLight.color", dirLight.color);
            shader->setFloat("u_DirectionalLight.intensity", dirLight.intensity);

            // Point lights
            uint32_t maxLights = 16;
            uint32_t pointCount = std::min(maxLights, (uint32_t)m_sceneData.sceneEnvironmentLight.pointLights.size());
            shader->setInt("u_PointLightCount", pointCount);
            for (uint32_t i = 0; i < pointCount; i++)
            {
                const auto &l = m_sceneData.sceneEnvironmentLight.pointLights[i];
                std::string base = "u_PointLights[" + std::to_string(i) + "]";
                shader->setFloat3(base + ".position", l.position);
                shader->setFloat3(base + ".color", l.color);
                shader->setFloat(base + ".intensity", l.intensity);
                shader->setFloat(base + ".range", l.range);
            }

            // Spot lights
            uint32_t spotCount = std::min(maxLights, (uint32_t)m_sceneData.sceneEnvironmentLight.spotLights.size());
            shader->setInt("u_SpotLightCount", spotCount);
            for (uint32_t i = 0; i < spotCount; i++)
            {
                const auto &l = m_sceneData.sceneEnvironmentLight.spotLights[i];
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
            shader->setFloat("u_NormalStrength", m_sceneData.normalMapStrength);
            shader->setFloat("u_ToksvigStrength", m_sceneData.toksvigStrength);

            if (cmd.material)
                cmd.material->bind(shader);

            RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
            m_renderer3DStatistics.geometryDrawCalls++;
        } });
    }

    void SceneRenderer::OutlinePass(ResourceHandle gBuffer, ResourceHandle sceneDepth, ResourceHandle lightingResult)
    {
        std::vector<int> outlineIDs = m_outlineIDs;
        bool hasOutlineDrawCommands = false;
        for (const auto &cmd : s_MeshDrawList)
        {
            if (cmd.drawOutline && cmd.visible)
            {
                hasOutlineDrawCommands = true;
                outlineIDs.push_back(cmd.objectID);
            }
        }

        std::vector<int> uniqueIDs;
        uniqueIDs.reserve(outlineIDs.size());
        const size_t maxOutlineIDs = 32;
        for (int id : outlineIDs)
        {
            if (id < 0)
                continue;

            bool exists = false;
            for (int existing : uniqueIDs)
            {
                if (existing == id)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
                uniqueIDs.push_back(id);

            if (uniqueIDs.size() >= maxOutlineIDs)
                break;
        }

        const bool canUseGBuffer = m_sceneData.renderMode == RenderMode::DeferredHybrid &&
                                   m_gBufferFramebuffer &&
                                   m_GBufferOutlinePipeline;

        if (canUseGBuffer && !uniqueIDs.empty())
        {
            m_RenderGraph.addPass(
                {.Name = "OutlinePass",
                 .Inputs = {gBuffer, sceneDepth, lightingResult},
                 .Execute = [this, outlineIDs = std::move(uniqueIDs)](CommandBuffer &commandBuffer)
                 {
                     recordOutlinePostProcess(commandBuffer, outlineIDs);
                 }});
            return;
        }

        if (!hasOutlineDrawCommands)
            return;

        m_RenderGraph.addPass(
            {.Name = "OutlinePass",
             .Inputs = {lightingResult},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 Renderer2D::recordOutlinePass(commandBuffer, s_MeshDrawList, m_sceneData.meshOutlineColor);
             }});
    }

    void SceneRenderer::recordOutlinePostProcess(CommandBuffer &commandBuffer, const std::vector<int> &outlineIDs)
    {
        commandBuffer.record([this, outlineIDs](RendererAPI &api)
                             {
            if (!m_gBufferFramebuffer || !m_GBufferOutlinePipeline || outlineIDs.empty())
                return;

            if (m_targetFramebuffer)
            {
                m_targetFramebuffer->bind();
            }
            else
            {
                uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
                uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
                if (viewportWidth > 0 && viewportHeight > 0)
                    RenderCommand::setViewport(0, 0, viewportWidth, viewportHeight);
            }

            m_GBufferOutlinePipeline->bind();
            auto shader = m_GBufferOutlinePipeline->getShader();

            shader->setInt("u_GBufferNormal", 0);
            shader->setInt("u_GBufferDepth", 1);
            shader->setInt("u_GBufferObjectID", 2);

            m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::Normal), 0);
            m_gBufferFramebuffer->bindDepthAttachment(1);
            m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(GBufferAttachment::ObjectID), 2);

            shader->setFloat4("u_OutlineColor", m_sceneData.meshOutlineColor);
            int outlineCount = static_cast<int>(outlineIDs.size());
            if (outlineCount > 32)
                outlineCount = 32;
            shader->setInt("u_OutlineIDCount", outlineCount);
            if (outlineCount > 0)
                shader->setIntArray("u_OutlineIDs", const_cast<int *>(outlineIDs.data()), outlineCount);

            shader->setFloat("u_Near", m_sceneData.sceneCamera.nearClip);
            shader->setFloat("u_Far", m_sceneData.sceneCamera.farClip);
            shader->setFloat("u_DepthThreshold", m_sceneData.outlineDepthThreshold);
            shader->setFloat("u_NormalThreshold", m_sceneData.outlineNormalThreshold);
            shader->setFloat("u_Thickness", m_sceneData.outlineThickness);

            RenderCommand::drawIndexed(m_depthViewQuadVA, m_depthViewQuadVA->getIndexBuffer()->getCount()); });
    }

    void SceneRenderer::SkyboxPass(ResourceHandle lightingResult)
    {
        if (!m_environmentRenderer)
            return;

        m_environmentRenderer->addSkyboxPass(m_RenderGraph,
                                             m_sceneData.sceneCamera.view,
                                             m_sceneData.sceneCamera.camera.getProjection(),
                                             &m_renderer3DStatistics.skyboxDrawCalls,
                                             lightingResult);
    }
    void SceneRenderer::ShadowPass(ResourceHandle shadowMap)
    {
        if (!m_shadowRenderer)
            return;

        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
        m_shadowRenderer->addPass(m_RenderGraph,
                                  shadowMap,
                                  s_MeshDrawList,
                                  m_sceneData.sceneEnvironmentLight.directionalLight,
                                  m_sceneData.shadowMapSize,
                                  m_targetFramebuffer,
                                  viewportWidth,
                                  viewportHeight,
                                  &m_renderer3DStatistics.shadowDrawCalls);
    }

    void SceneRenderer::DepthViewPass(ResourceHandle sceneDepth, ResourceHandle lightingResult)
    {
        if (!m_targetFramebuffer)
            return;

        m_RenderGraph.addPass(
            {.Name = "DepthViewPass",
             .Inputs = {sceneDepth, lightingResult},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 commandBuffer.record([this](RendererAPI &api)
                                      {
                    m_DepthViewPipeline->bind();
                    auto shader = m_DepthViewPipeline->getShader();

                    shader->setInt("u_Depth", 0);
                    if (m_gBufferFramebuffer && m_sceneData.renderMode == RenderMode::DeferredHybrid)
                    {
                        m_gBufferFramebuffer->bindDepthAttachment(0);
                    }
                    else
                    {
                        m_targetFramebuffer->bindDepthAttachment(0);
                    }

                    shader->setFloat("u_Near", m_sceneData.sceneCamera.nearClip);
                    shader->setFloat("u_Far", m_sceneData.sceneCamera.farClip);
                    shader->setInt("u_IsPerspective", 1);

                    shader->setFloat("u_Power", m_sceneData.depthViewPower);
                    
                    RenderCommand::drawIndexed(m_depthViewQuadVA, m_depthViewQuadVA->getIndexBuffer()->getCount()); });
             }});
    }

    void SceneRenderer::ensureGBuffer(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (m_gBufferFramebuffer &&
            m_gBufferFramebuffer->getSpecification().width == width &&
            m_gBufferFramebuffer->getSpecification().height == height)
        {
            return;
        }

        FramebufferSpecification gBufferSpec;
        gBufferSpec.width = width;
        gBufferSpec.height = height;
        gBufferSpec.attachments = {
            FramebufferTextureFormat::RGBA8,       // Albedo
            FramebufferTextureFormat::RGB16F,      // Normal
            FramebufferTextureFormat::RGB16F,      // Material (Roughness/Metallic/AO)
            FramebufferTextureFormat::RGB16F,      // Emissive
            FramebufferTextureFormat::RED_INTEGER, // ObjectID
            FramebufferTextureFormat::Depth        // Depth
        };
        gBufferSpec.swapChainTarget = false;

        m_gBufferFramebuffer = Framebuffer::create(gBufferSpec);
    }

    void SceneRenderer::ensureSSGI(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        const bool hasFramebuffers = m_ssgiFramebuffers[0] && m_ssgiFramebuffers[1];
        if (hasFramebuffers &&
            m_ssgiFramebuffers[0]->getSpecification().width == width &&
            m_ssgiFramebuffers[0]->getSpecification().height == height)
            return;

        FramebufferSpecification ssgiSpec;
        ssgiSpec.width = width;
        ssgiSpec.height = height;
        ssgiSpec.attachments = {
            FramebufferTextureFormat::RGB16F};
        ssgiSpec.swapChainTarget = false;

        m_ssgiFramebuffers[0] = Framebuffer::create(ssgiSpec);
        m_ssgiFramebuffers[1] = Framebuffer::create(ssgiSpec);
        m_ssgiFramebuffer = m_ssgiFramebuffers[0];
        m_ssgiHistoryIndex = 0;
        m_ssgiFrameIndex = 0;
        m_ssgiHistoryValid = false;
        m_ssgiWasEnabled = false;
    }

    void SceneRenderer::ensureGTAO(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (m_gtaoFramebuffer &&
            m_gtaoFramebuffer->getSpecification().width == width &&
            m_gtaoFramebuffer->getSpecification().height == height)
        {
            return;
        }

        FramebufferSpecification gtaoSpec;
        gtaoSpec.width = width;
        gtaoSpec.height = height;
        gtaoSpec.attachments = {
            FramebufferTextureFormat::RG16F};
        gtaoSpec.swapChainTarget = false;

        m_gtaoFramebuffer = Framebuffer::create(gtaoSpec);
    }

    void SceneRenderer::FlushDrawList()
    {
        m_RenderGraph.reset();

        m_renderer3DStatistics.meshCount += static_cast<uint32_t>(s_MeshDrawList.size());

        ResourceHandle shadowMap = ResourceHandle{0};
        if (m_sceneData.enableShadows)
            shadowMap = RenderGraph::createResource();

        ResourceHandle gBuffer = RenderGraph::createResource();
        ResourceHandle lightingResult = RenderGraph::createResource();
        ResourceHandle sceneDepth = RenderGraph::createResource();
        ResourceHandle ssgi = ResourceHandle{0};
        ResourceHandle gtao = ResourceHandle{0};

        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
        const bool useDeferred = m_sceneData.renderMode == RenderMode::DeferredHybrid;
        if (useDeferred)
            ensureGBuffer(viewportWidth, viewportHeight);
        const bool useSSGI = useDeferred && (m_sceneData.enableSSGI || m_sceneData.gbufferDebug == GBufferDebugMode::SSGI);
        if (useSSGI)
        {
            ensureSSGI(viewportWidth, viewportHeight);
            ssgi = RenderGraph::createResource();
        }
        else
        {
            m_ssgiWasEnabled = false;
            m_ssgiHistoryValid = false;
            m_ssgiFrameIndex = 0;
        }
        const bool useGTAO = useDeferred && (m_sceneData.enableGTAO || m_sceneData.gbufferDebug == GBufferDebugMode::GTAO);
        if (useGTAO)
        {
            ensureGTAO(viewportWidth, viewportHeight);
            gtao = RenderGraph::createResource();
        }

        bool hasTransparent = false;
        for (const auto &cmd : s_MeshDrawList)
        {
            if (cmd.transparent)
            {
                hasTransparent = true;
                break;
            }
        }

        if (m_sceneData.enableShadows)
            ShadowPass(shadowMap);

        const bool showGBufferDebug = useDeferred && (m_sceneData.gbufferDebug != GBufferDebugMode::None);

        if (useDeferred)
        {
            GBufferPass(gBuffer, sceneDepth);

            if (useSSGI)
                SSGIPass(gBuffer, sceneDepth, ssgi);
            if (useGTAO)
                GTAOPass(gBuffer, sceneDepth, gtao);

            if (showGBufferDebug)
            {
                GBufferDebugPass(gBuffer, sceneDepth, ssgi, gtao);
            }
            else
            {
                LightingPass(gBuffer, shadowMap, sceneDepth, ssgi, gtao, lightingResult);

                if (m_sceneData.showSkybox)
                    SkyboxPass(lightingResult);

                if (hasTransparent)
                    TransparentPass(shadowMap, sceneDepth, lightingResult);
            }
        }
        else
        {
            ForwardPass(shadowMap, sceneDepth, lightingResult);

            if (m_sceneData.showSkybox)
                SkyboxPass(lightingResult);

            if (hasTransparent)
                TransparentPass(shadowMap, sceneDepth, lightingResult);
        }

        if (m_sceneData.enableDepthView && !showGBufferDebug)
            DepthViewPass(sceneDepth, lightingResult);

        OutlinePass(gBuffer, sceneDepth, lightingResult);

        RendererBackend backend(RenderCommand::GetRendererAPI());
        m_RenderGraph.execute(m_CommandQueue, backend);
        s_MeshDrawList.clear();
        m_outlineIDs.clear();
    }

    void SceneRenderer::loadHDREnvironment(const std::string &hdrPath)
    {
        if (!m_environmentRenderer)
            return;

        uint32_t viewportWidth = m_scene ? m_scene->getViewportWidth() : 0;
        uint32_t viewportHeight = m_scene ? m_scene->getViewportHeight() : 0;
        m_environmentRenderer->loadHDR(hdrPath, m_targetFramebuffer, viewportWidth, viewportHeight);
    }

} // namespace Fermion
