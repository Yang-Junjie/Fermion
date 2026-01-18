#include "SceneRenderer.hpp"
#include "Renderer2D.hpp"
#include "Renderer3D.hpp"
#include "Renderer/RendererBackend.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Model/MeshFactory.hpp"
#include "Project/Project.hpp"
#include "Renderer.hpp"

namespace Fermion
{
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

        // Skybox Pipeline
        {
            PipelineSpecification skyboxSpec;
            skyboxSpec.shader = Renderer::getShaderLibrary()->get("Skybox");
            skyboxSpec.depthTest = true;
            skyboxSpec.depthWrite = false;
            skyboxSpec.depthOperator = DepthCompareOperator::LessOrEqual;
            skyboxSpec.cull = CullMode::None;

            m_SkyboxPipeline = Pipeline::create(skyboxSpec);
        }

        // Shadow Pipeline
        {
            PipelineSpecification shadowSpec;
            shadowSpec.shader = Renderer::getShaderLibrary()->get("Shadow");
            shadowSpec.depthTest = true;
            shadowSpec.depthWrite = true;
            shadowSpec.depthOperator = DepthCompareOperator::Less;
            shadowSpec.cull = CullMode::Back;

            m_ShadowPipeline = Pipeline::create(shadowSpec);
        }

        // IBL Pipelines
        {
            PipelineSpecification iblIrradianceSpec;
            iblIrradianceSpec.shader = Renderer::getShaderLibrary()->get("IBLPreprocess");
            iblIrradianceSpec.depthTest = false;
            iblIrradianceSpec.depthWrite = false;
            iblIrradianceSpec.cull = CullMode::None;
            m_IBLIrradiancePipeline = Pipeline::create(iblIrradianceSpec);

            PipelineSpecification iblPrefilterSpec;
            iblPrefilterSpec.shader = Renderer::getShaderLibrary()->get("IBLPrefilter");
            iblPrefilterSpec.depthTest = false;
            iblPrefilterSpec.depthWrite = false;
            iblPrefilterSpec.cull = CullMode::None;
            m_IBLPrefilterPipeline = Pipeline::create(iblPrefilterSpec);

            PipelineSpecification iblBRDFSpec;
            iblBRDFSpec.shader = Renderer::getShaderLibrary()->get("BRDFLUT");
            iblBRDFSpec.depthTest = false;
            iblBRDFSpec.depthWrite = false;
            iblBRDFSpec.cull = CullMode::None;
            m_IBLBRDFPipeline = Pipeline::create(iblBRDFSpec);

            PipelineSpecification equirectToCubeSpec;
            equirectToCubeSpec.shader = Renderer::getShaderLibrary()->get("EquirectToCube");
            equirectToCubeSpec.depthTest = false;
            equirectToCubeSpec.depthWrite = false;
            equirectToCubeSpec.cull = CullMode::None;
            m_EquirectToCubePipeline = Pipeline::create(equirectToCubeSpec);
        }

        // DepthView Pipeline
        {
            PipelineSpecification depthViewSpec;
            depthViewSpec.shader = Renderer::getShaderLibrary()->get("DepthView");
            depthViewSpec.depthTest = false;
            depthViewSpec.depthWrite = false;
            depthViewSpec.cull = CullMode::None;
            m_DepthViewPipeline = Pipeline::create(depthViewSpec);
        }

        // Shadow Map Framebuffer
        {
            FramebufferSpecification shadowFBSpec;
            shadowFBSpec.width = m_sceneData.shadowMapSize;
            shadowFBSpec.height = m_sceneData.shadowMapSize;
            shadowFBSpec.attachments = {FramebufferTextureFormat::DEPTH_COMPONENT32F};
            shadowFBSpec.swapChainTarget = false;

            m_shadowMapFB = Framebuffer::create(shadowFBSpec);
        }

        // Skybox VAO
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

        m_cubeVA = VertexArray::create();
        m_cubeVA->addVertexBuffer(vertexBuffer);
        m_cubeVA->setIndexBuffer(indexBuffer);

        // Fullscreen quad for BRDF LUT generation
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

        m_quadVA = VertexArray::create();
        m_quadVA->addVertexBuffer(quadVB);
        m_quadVA->setIndexBuffer(quadIB);

        // DepthView quad VAO (reuse the same quad geometry)
        m_depthViewQuadVA = m_quadVA;

        loadHDREnvironment(m_sceneData.hdrPath);
    }

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
            if (cmd.drawOutline)
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
                 
                 for (auto &cmd : s_MeshDrawList) {
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
                         
                         // IBL
                         if (!m_iblInitialized && m_sceneData.useIBL) {
                             initializeIBL();
                         }
                         
                         shader->setBool("u_UseIBL", m_sceneData.useIBL && m_iblInitialized);
                         if (m_sceneData.useIBL && m_iblInitialized) {
                             shader->setInt("u_IrradianceMap", 11);
                             shader->setInt("u_PrefilterMap", 12);
                             shader->setInt("u_BRDFLT", 13);
                             shader->setFloat("u_PrefilterMaxLOD", static_cast<float>(m_sceneData.prefilterMaxMipLevels - 1));
                             
                             // Bind IBL textures
                             m_irradianceMap->bind(11);
                             m_prefilterMap->bind(12);
                             m_brdfLUT->bind(13);
                         }
                     }
                         
                         // Shadow mapping
                         shader->setBool("u_EnableShadows", m_sceneData.enableShadows);
                         if (m_sceneData.enableShadows) {
                             shader->setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);
                             shader->setFloat("u_ShadowBias", m_sceneData.shadowBias);
                             shader->setFloat("u_ShadowSoftness", m_sceneData.shadowSoftness);
                             shader->setInt("u_ShadowMap", 10); 
                             

                             m_shadowMapFB->bindDepthAttachment(10);
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
        m_RenderGraph.addPass(
            {.Name = "SkyboxPass",
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 if (!m_environmentCubemap)
                     return;

                 SkyboxDrawCommand cmd;
                 cmd.pipeline = m_SkyboxPipeline;
                 cmd.vao = m_cubeVA;
                 cmd.cubemap = m_environmentCubemap.get();
                 cmd.view = m_sceneData.sceneCamera.view;
                 cmd.projection = m_sceneData.sceneCamera.camera.getProjection();

                 if (cmd.pipeline && cmd.vao && cmd.cubemap)
                     m_renderer3DStatistics.skyboxDrawCalls++;

                 Renderer3D::recordSkyboxPass(commandBuffer, cmd);
             }});
    }
    void SceneRenderer::ShadowPass(ResourceHandle shadowMap)
    {
        if (m_shadowMapFB->getSpecification().width != m_sceneData.shadowMapSize)
        {
            FramebufferSpecification shadowFBSpec;
            shadowFBSpec.width = m_sceneData.shadowMapSize;
            shadowFBSpec.height = m_sceneData.shadowMapSize;
            shadowFBSpec.attachments = {FramebufferTextureFormat::DEPTH_COMPONENT32F};
            shadowFBSpec.swapChainTarget = false;

            m_shadowMapFB = Framebuffer::create(shadowFBSpec);
        }

        m_lightSpaceMatrix = calculateLightSpaceMatrix(m_sceneData.sceneEnvironmentLight.directionalLight);

        m_RenderGraph.addPass(
            {.Name = "ShadowPass",
             .Outputs = {shadowMap},
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 commandBuffer.record([this](RendererAPI &api)
                                      {
                     m_shadowMapFB->bind();
                     RenderCommand::clear();
                     
                     m_ShadowPipeline->bind();
                     auto shader = m_ShadowPipeline->getShader();
                     shader->setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);
                     
                      for (auto &cmd : s_MeshDrawList) {
                          shader->setMat4("u_Model", cmd.transform);
                          RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                          m_renderer3DStatistics.shadowDrawCalls++;
                      }
                      
                      if (m_targetFramebuffer) {
                          m_targetFramebuffer->bind();
                      } else {
                          m_shadowMapFB->unbind();
                          if (m_scene && m_scene->m_viewportWidth > 0 && m_scene->m_viewportHeight > 0)
                              RenderCommand::setViewport(0, 0, m_scene->m_viewportWidth, m_scene->m_viewportHeight);
                      } });
             }});
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

    glm::mat4 SceneRenderer::calculateLightSpaceMatrix(const DirectionalLight &light, float orthoSize)
    {
        glm::vec3 lightDir = glm::normalize(light.direction);
        glm::vec3 lightPos = lightDir * orthoSize;

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::abs(glm::dot(lightDir, up)) > 0.99f)
        {
            up = glm::vec3(1.0f, 0.0f, 0.0f);
        }

        glm::mat4 lightView = glm::lookAt(
            lightPos,
            glm::vec3(0.0f, 0.0f, 0.0f),
            up);

        glm::mat4 lightProjection = glm::ortho(
            -orthoSize, orthoSize,
            -orthoSize, orthoSize,
            0.1f, orthoSize * 3.0f);

        return lightProjection * lightView;
    }

    void SceneRenderer::loadHDREnvironment(const std::string &hdrPath)
    {
        Log::Info(std::format("Loading HDR environment: {}", hdrPath));

        m_hdrEnvironment = Texture2D::create(hdrPath);
        if (!m_hdrEnvironment || !m_hdrEnvironment->isLoaded())
        {
            Log::Error(std::format("Failed to load HDR environment from: {}", hdrPath));
            return;
        }

        Log::Info(std::format("HDR loaded: {}x{}", m_hdrEnvironment->getWidth(), m_hdrEnvironment->getHeight()));

        convertEquirectangularToCubemap();

        m_environmentLoaded = true;
        m_iblInitialized = false;

        Log::Info("HDR environment loaded successfully");
    }

    void SceneRenderer::convertEquirectangularToCubemap()
    {
        if (!m_hdrEnvironment)
        {
            Log::Error("No HDR environment loaded for conversion");
            return;
        }

        Log::Info("Converting equirectangular HDR to cubemap...");

        // 创建目标cubemap
        uint32_t cubemapSize = 4096;
        TextureCubeSpecification cubemapSpec;
        cubemapSpec.width = cubemapSize;
        cubemapSpec.height = cubemapSize;
        cubemapSpec.format = ImageFormat::RGB16F;
        cubemapSpec.generateMips = true;
        cubemapSpec.maxMipLevels = 5;
        m_environmentCubemap = TextureCube::create(cubemapSpec);

        // 创建帧缓冲用于渲染
        FramebufferSpecification fbSpec;
        fbSpec.width = cubemapSize;
        fbSpec.height = cubemapSize;
        fbSpec.attachments = {FramebufferTextureFormat::RGB16F};
        fbSpec.swapChainTarget = false;
        auto captureFB = Framebuffer::create(fbSpec);

        // 设置投影和视图矩阵
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        m_EquirectToCubePipeline->bind();
        auto shader = m_EquirectToCubePipeline->getShader();
        shader->setInt("u_EquirectangularMap", 0);
        shader->setMat4("u_Projection", captureProjection);
        m_hdrEnvironment->bind(0);

        // 渲染6个面
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

        // 恢复原始viewport
        if (m_targetFramebuffer)
        {
            m_targetFramebuffer->bind();
        }
        else
        {
            if (m_scene && m_scene->m_viewportWidth > 0 && m_scene->m_viewportHeight > 0)
                RenderCommand::setViewport(0, 0, m_scene->m_viewportWidth, m_scene->m_viewportHeight);
        }

        Log::Info("Equirectangular to cubemap conversion completed");
    }

    void SceneRenderer::initializeIBL()
    {
        if (m_iblInitialized || !m_environmentCubemap)
            return;

        Log::Info("Initializing IBL from HDR environment...");

        generateIrradianceMap();
        generatePrefilterMap();
        generateBRDFLUT();

        m_iblInitialized = true;
        Log::Info("IBL initialization completed");
    }

    void SceneRenderer::generateIrradianceMap()
    {
        Log::Info(std::format("Generating irradiance map (size: {}x{})...",
                              m_sceneData.irradianceMapSize, m_sceneData.irradianceMapSize));

        // 创建辐照度贴图
        TextureCubeSpecification irradianceSpec;
        irradianceSpec.width = m_sceneData.irradianceMapSize;
        irradianceSpec.height = m_sceneData.irradianceMapSize;
        irradianceSpec.format = ImageFormat::RGB16F;
        irradianceSpec.generateMips = false;
        irradianceSpec.maxMipLevels = 1;
        m_irradianceMap = TextureCube::create(irradianceSpec);

        // 创建帧缓冲用于渲染
        FramebufferSpecification fbSpec;
        fbSpec.width = m_sceneData.irradianceMapSize;
        fbSpec.height = m_sceneData.irradianceMapSize;
        fbSpec.attachments = {FramebufferTextureFormat::RGB16F};
        fbSpec.swapChainTarget = false;
        auto captureFB = Framebuffer::create(fbSpec);

        // 设置投影矩阵
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        m_IBLIrradiancePipeline->bind();
        auto shader = m_IBLIrradiancePipeline->getShader();
        shader->setInt("u_EnvironmentMap", 0);
        shader->setMat4("u_Projection", captureProjection);
        m_environmentCubemap->bind(0);

        const char *faceNames[] = {"+X (Right)", "-X (Left)", "+Y (Up)", "-Y (Down)", "+Z (Front)", "-Z (Back)"};

        for (uint32_t i = 0; i < 6; ++i)
        {
            shader->setMat4("u_View", captureViews[i]);
            captureFB->bind();
            RenderCommand::setViewport(0, 0, m_sceneData.irradianceMapSize, m_sceneData.irradianceMapSize);
            RenderCommand::clear();
            RenderCommand::drawIndexed(m_cubeVA, m_cubeVA->getIndexBuffer()->getCount());
            m_renderer3DStatistics.iblDrawCalls++;

            // 将渲染结果复制到cubemap的对应面
            m_irradianceMap->copyFromFramebuffer(captureFB, i, 0);
        }

        captureFB->unbind();

        // 恢复原始framebuffer
        if (m_targetFramebuffer)
        {
            m_targetFramebuffer->bind();
        }
        else
        {
            if (m_scene && m_scene->m_viewportWidth > 0 && m_scene->m_viewportHeight > 0)
                RenderCommand::setViewport(0, 0, m_scene->m_viewportWidth, m_scene->m_viewportHeight);
        }

        Log::Info("Irradiance map generation completed");
    }

    void SceneRenderer::generatePrefilterMap()
    {
        // 创建预过滤贴图
        TextureCubeSpecification prefilterSpec;
        prefilterSpec.width = m_sceneData.prefilterMapSize;
        prefilterSpec.height = m_sceneData.prefilterMapSize;
        prefilterSpec.format = ImageFormat::RGB16F;
        prefilterSpec.generateMips = true;
        prefilterSpec.maxMipLevels = m_sceneData.prefilterMaxMipLevels;
        m_prefilterMap = TextureCube::create(prefilterSpec);

        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        m_IBLPrefilterPipeline->bind();
        auto shader = m_IBLPrefilterPipeline->getShader();
        shader->setInt("u_EnvironmentMap", 0);
        shader->setMat4("u_Projection", captureProjection);
        m_environmentCubemap->bind(0);

        // 为每个mip级别生成预过滤贴图
        for (uint32_t mip = 0; mip < m_sceneData.prefilterMaxMipLevels; ++mip)
        {
            uint32_t mipWidth = static_cast<uint32_t>(m_sceneData.prefilterMapSize * std::pow(0.5, mip));
            uint32_t mipHeight = mipWidth;

            FramebufferSpecification fbSpec;
            fbSpec.width = mipWidth;
            fbSpec.height = mipHeight;
            fbSpec.attachments = {FramebufferTextureFormat::RGB16F};
            fbSpec.swapChainTarget = false;
            auto captureFB = Framebuffer::create(fbSpec);

            float roughness = static_cast<float>(mip) / static_cast<float>(m_sceneData.prefilterMaxMipLevels - 1);
            shader->setFloat("u_Roughness", roughness);

            for (uint32_t i = 0; i < 6; ++i)
            {
                shader->setMat4("u_View", captureViews[i]);
                captureFB->bind();
                RenderCommand::setViewport(0, 0, mipWidth, mipHeight);
                RenderCommand::clear();
                RenderCommand::drawIndexed(m_cubeVA, m_cubeVA->getIndexBuffer()->getCount());
                m_renderer3DStatistics.iblDrawCalls++;

                m_prefilterMap->copyFromFramebuffer(captureFB, i, mip);
            }

            captureFB->unbind();
        }

        if (m_targetFramebuffer)
        {
            m_targetFramebuffer->bind();
        }
        else
        {
            if (m_scene && m_scene->m_viewportWidth > 0 && m_scene->m_viewportHeight > 0)
                RenderCommand::setViewport(0, 0, m_scene->m_viewportWidth, m_scene->m_viewportHeight);
        }
    }

    void SceneRenderer::generateBRDFLUT()
    {
        // 创建BRDF LUT纹理
        TextureSpecification brdfSpec;
        brdfSpec.Width = m_sceneData.brdfLUTSize;
        brdfSpec.Height = m_sceneData.brdfLUTSize;
        brdfSpec.Format = ImageFormat::RG16F;
        brdfSpec.GenerateMips = false;
        m_brdfLUT = Texture2D::create(brdfSpec);

        // 创建帧缓冲
        FramebufferSpecification fbSpec;
        fbSpec.width = m_sceneData.brdfLUTSize;
        fbSpec.height = m_sceneData.brdfLUTSize;
        fbSpec.attachments = {FramebufferTextureFormat::RG16F};
        fbSpec.swapChainTarget = false;
        auto captureFB = Framebuffer::create(fbSpec);

        captureFB->bind();
        RenderCommand::setViewport(0, 0, m_sceneData.brdfLUTSize, m_sceneData.brdfLUTSize);

        m_IBLBRDFPipeline->bind();
        RenderCommand::clear();

        RenderCommand::drawIndexed(m_quadVA, m_quadVA->getIndexBuffer()->getCount());
        m_renderer3DStatistics.iblDrawCalls++;

        // 复制到纹理
        m_brdfLUT->copyFromFramebuffer(captureFB, 0, 0);

        captureFB->unbind();

        if (m_targetFramebuffer)
        {
            m_targetFramebuffer->bind();
        }
        else
        {
            if (m_scene && m_scene->m_viewportWidth > 0 && m_scene->m_viewportHeight > 0)
                RenderCommand::setViewport(0, 0, m_scene->m_viewportWidth, m_scene->m_viewportHeight);
        }
    }

    uint32_t SceneRenderer::getDepthViewRendererID() const
    {
        // 不再需要此方法，因为直接渲染到 viewport
        return 0;
    }
} // namespace Fermion
