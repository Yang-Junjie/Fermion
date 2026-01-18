#include "SceneRenderer.hpp"
#include "Renderer2D.hpp"
#include "Renderer3D.hpp"
#include "Renderer/RendererBackend.hpp"
#include "Renderer/RenderCommand.hpp"
#include "EnvironmentRenderer.hpp"
#include "ShadowMapRenderer.hpp"
#include "Project/Project.hpp"
#include "Renderer.hpp"
#include <cfloat>

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

    }
    SceneRenderer::SceneRenderer()
    {
        m_debugRenderer = std::make_shared<DebugRenderer>();

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
        Renderer3D::updateViewState(camera.camera, camera.view, m_sceneData.sceneEnvironmentLight);
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

    void SceneRenderer::GeometryPass(ResourceHandle shadowMap, ResourceHandle sceneDepth)
    {
        m_RenderGraph.addPass(
            {.Name = "GeometryPass",
             .Inputs = {shadowMap},
             .Outputs = {sceneDepth},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 commandBuffer.record([this](RendererAPI &api)
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
                 
                 for (auto &cmd : s_MeshDrawList) {
                     if (!cmd.visible)
                         continue;
                     if (currentPipeline != cmd.pipeline) {
                         currentPipeline = cmd.pipeline;
                         currentPipeline->bind();
                     }
                     
                     auto shader = currentPipeline->getShader();
                     shader->setMat4("u_Model", cmd.transform);
                     shader->setInt("u_ObjectID", cmd.objectID);
                     
                     if (cmd.pipeline == m_PBRMeshPipeline) {
                         glm::vec3 cameraPos = glm::vec3(glm::inverse(m_sceneData.sceneCamera.view)[3]);
                         shader->setFloat3("u_CameraPosition", cameraPos);
                         shader->setFloat("u_AmbientIntensity", m_sceneData.ambientIntensity);
                        
                         if (m_environmentRenderer) {
                             m_environmentRenderer->ensureIBLInitialized(iblSettings, m_targetFramebuffer, viewportWidth, viewportHeight,
                                                                          &m_renderer3DStatistics.iblDrawCalls);
                             m_environmentRenderer->bindIBL(shader, iblSettings);
                         } else {
                             shader->setBool("u_UseIBL", false);
                         }
                     }
                         
                         // Shadow mapping
                         bool enableShadows = m_sceneData.enableShadows && m_shadowRenderer && m_shadowRenderer->getShadowMapFramebuffer();
                         shader->setBool("u_EnableShadows", enableShadows);
                         if (enableShadows) {
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
                         for (uint32_t i = 0; i < pointCount; i++) {
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
                         for (uint32_t i = 0; i < spotCount; i++) {
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
                         
                         if (cmd.material)
                             cmd.material->bind(shader);
                             
                         RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                         m_renderer3DStatistics.geometryDrawCalls++;
                     } });
             }});
    }

    void SceneRenderer::OutlinePass()
    {
        m_RenderGraph.addPass(
            {.Name = "OutlinePass",
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 Renderer2D::recordOutlinePass(commandBuffer, s_MeshDrawList, m_sceneData.meshOutlineColor);
             }});
    }

    void SceneRenderer::SkyboxPass()
    {
        if (!m_environmentRenderer)
            return;

        m_environmentRenderer->addSkyboxPass(m_RenderGraph,
                                             m_sceneData.sceneCamera.view,
                                             m_sceneData.sceneCamera.camera.getProjection(),
                                             &m_renderer3DStatistics.skyboxDrawCalls);
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

    void SceneRenderer::DepthViewPass(ResourceHandle sceneDepth)
    {
        if (!m_targetFramebuffer)
            return;

        m_RenderGraph.addPass(
            {.Name = "DepthViewPass",
             .Inputs = {sceneDepth},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 commandBuffer.record([this](RendererAPI &api)
                                      {
                    m_DepthViewPipeline->bind();
                    auto shader = m_DepthViewPipeline->getShader();

                    shader->setInt("u_Depth", 0);
                    m_targetFramebuffer->bindDepthAttachment(0);

                    shader->setFloat("u_Near", m_sceneData.sceneCamera.nearClip);
                    shader->setFloat("u_Far", m_sceneData.sceneCamera.farClip);
                    shader->setInt("u_IsPerspective", 1);

                    shader->setFloat("u_Power", m_sceneData.depthViewPower);
                    
                    RenderCommand::drawIndexed(m_depthViewQuadVA, m_depthViewQuadVA->getIndexBuffer()->getCount()); });
             }});
    }

    void SceneRenderer::FlushDrawList()
    {
        m_RenderGraph.reset();

        m_renderer3DStatistics.meshCount += static_cast<uint32_t>(s_MeshDrawList.size());

        ResourceHandle shadowMap = ResourceHandle{0};
        if (m_sceneData.enableShadows)
            shadowMap = RenderGraph::createResource();

        ResourceHandle sceneDepth = RenderGraph::createResource();

        if (m_sceneData.enableShadows)
            ShadowPass(shadowMap);

        if (m_sceneData.showSkybox)
            SkyboxPass();
        OutlinePass();

        GeometryPass(shadowMap, sceneDepth);

        if (m_sceneData.enableDepthView)
            DepthViewPass(sceneDepth);

        RendererBackend backend(RenderCommand::GetRendererAPI());
        m_RenderGraph.execute(m_CommandQueue, backend);
        s_MeshDrawList.clear();
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
