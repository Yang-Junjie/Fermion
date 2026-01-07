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

        // Skybox create
        {
            TextureCubeSpecification spec;
            spec.flip = false;
            spec.path = "../Boson/projects/Assets/textures/skybox2";
            spec.names[TextureCubeFace::Front] = "posz.jpg";
            spec.names[TextureCubeFace::Back] = "negz.jpg";
            spec.names[TextureCubeFace::Left] = "negx.jpg";
            spec.names[TextureCubeFace::Right] = "posx.jpg";
            spec.names[TextureCubeFace::Up] = "posy.jpg";
            spec.names[TextureCubeFace::Down] = "negy.jpg";

            m_skybox = TextureCube::create(spec);
        }
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
    }

    void SceneRenderer::beginScene(const Camera &camera, const glm::mat4 &transform)
    {
        beginScene({camera, glm::inverse(transform)});
    }

    void SceneRenderer::beginScene(const EditorCamera &camera)
    {
        beginScene({camera, camera.getViewMatrix()});
    }

    void SceneRenderer::beginScene(const SceneRendererCamera &camera)
    {
        m_sceneData.sceneCamera = camera;
        m_sceneData.sceneEnvironmentLight = m_scene->m_environmentLight;
        Renderer2D::beginScene(camera.camera, camera.view);
        Renderer3D::updateViewState(camera.camera, camera.view, m_sceneData.sceneEnvironmentLight);
    }

    void SceneRenderer::endScene()
    {
        FlushDrawList();
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
            auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
            if (mesh)
            {
                for (auto &submesh : mesh->getSubMeshes())
                {
                    MeshDrawCommand cmd;
                    cmd.pipeline = m_MeshPipeline;
                    cmd.vao = mesh->getVertexArray();
                    cmd.material = mesh->getMaterials()[submesh.MaterialIndex];
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

    void SceneRenderer::submitMesh(MeshComponent &meshComponent, PBRMaterialComponent &pbrMaterial,
                                   glm::mat4 transform, int objectId, bool drawOutline)
    {
        if (static_cast<uint64_t>(meshComponent.meshHandle) != 0)
        {
            auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
            if (mesh)
            {
                for (auto &submesh : mesh->getSubMeshes())
                {
                    MeshDrawCommand cmd;
                    cmd.pipeline = m_PBRMeshPipeline;
                    cmd.vao = mesh->getVertexArray();

                    auto material = mesh->getMaterials()[submesh.MaterialIndex];
                    if (!material)
                    {
                        material = std::make_shared<Material>();
                    }

                    material->setMaterialType(MaterialType::PBR);
                    material->setAlbedo(pbrMaterial.albedo);
                    material->setMetallic(pbrMaterial.metallic);
                    material->setRoughness(pbrMaterial.roughness);
                    material->setAO(pbrMaterial.ao);

                    auto assetManager = Project::getRuntimeAssetManager();

                    if (static_cast<uint64_t>(pbrMaterial.albedoMapHandle) != 0)
                    {
                        auto texture = assetManager->getAsset<Texture2D>(pbrMaterial.albedoMapHandle);
                        if (texture)
                        {
                            material->setAlbedoMapShared(texture);
                        }
                    }

                    if (static_cast<uint64_t>(pbrMaterial.normalMapHandle) != 0)
                    {
                        auto texture = assetManager->getAsset<Texture2D>(pbrMaterial.normalMapHandle);
                        if (texture)
                        {
                            material->setNormalMapShared(texture);
                        }
                    }

                    if (static_cast<uint64_t>(pbrMaterial.metallicMapHandle) != 0)
                    {
                        auto texture = assetManager->getAsset<Texture2D>(pbrMaterial.metallicMapHandle);
                        if (texture)
                        {
                            material->setMetallicMapShared(texture);
                        }
                    }

                    if (static_cast<uint64_t>(pbrMaterial.roughnessMapHandle) != 0)
                    {
                        auto texture = assetManager->getAsset<Texture2D>(pbrMaterial.roughnessMapHandle);
                        if (texture)
                        {
                            material->setRoughnessMapShared(texture);
                        }
                    }

                    if (static_cast<uint64_t>(pbrMaterial.aoMapHandle) != 0)
                    {
                        auto texture = assetManager->getAsset<Texture2D>(pbrMaterial.aoMapHandle);
                        if (texture)
                        {
                            material->setAOMapShared(texture);
                        }
                    }

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

    void SceneRenderer::submitMesh(MeshComponent &meshComponent, PhongMaterialComponent &phongMaterial,
                                   glm::mat4 transform, int objectId, bool drawOutline)
    {
        if (static_cast<uint64_t>(meshComponent.meshHandle) != 0)
        {
            auto mesh = Project::getRuntimeAssetManager()->getAsset<Mesh>(meshComponent.meshHandle);
            if (mesh)
            {
                for (auto &submesh : mesh->getSubMeshes())
                {
                    MeshDrawCommand cmd;
                    // 使用Phong管线
                    cmd.pipeline = m_MeshPipeline;
                    cmd.vao = mesh->getVertexArray();

                    // 创建Phong材质并加载纹理
                    auto material = mesh->getMaterials()[submesh.MaterialIndex];
                    if (!material)
                    {
                        material = std::make_shared<Material>();
                    }

                    material->setMaterialType(MaterialType::Phong);
                    material->setDiffuseColor(phongMaterial.diffuseColor);
                    material->setAmbientColor(phongMaterial.ambientColor);

                    // 从AssetHandle加载漫反射纹理
                    if (phongMaterial.useTexture && static_cast<uint64_t>(phongMaterial.diffuseTextureHandle) != 0)
                    {
                        auto texture = Project::getRuntimeAssetManager()->getAsset<Texture2D>(phongMaterial.diffuseTextureHandle);
                        if (texture)
                        {
                            material->setTextureShared(texture);
                        }
                    }

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
    void SceneRenderer::drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color)
    {
        Renderer2D::drawLine(start, end, color);
    }

    void SceneRenderer::setLineWidth(float thickness)
    {
        Renderer2D::setLineWidth(thickness);
    }

    SceneRenderer::Statistics SceneRenderer::getStatistics() const
    {
        Renderer2D::Satistics stats2D = Renderer2D::getStatistics();
        Statistics result;
        result.drawCalls = stats2D.drawCalls;
        result.quadCount = stats2D.quadCount;
        result.lineCount = stats2D.lineCount;
        result.circleCount = stats2D.circleCount;
        return result;
    }

    void SceneRenderer::GeometryPass()
    {
        m_RenderGraph.AddPass(
            {.Name = "GeometryPass",
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 commandBuffer.Record([this](RendererAPI &api)
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
                         shader->setFloat("u_AmbientIntensity", 0.03f); 
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
                         
                         if (cmd.material)
                             cmd.material->bind(shader);
                             
                         RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                     } });
             }});
    }

    void SceneRenderer::OutlinePass()
    {
        m_RenderGraph.AddPass(
            {.Name = "OutlinePass",
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 Renderer2D::recordOutlinePass(commandBuffer, s_MeshDrawList, m_sceneData.meshOutlineColor);
             }});
    }

    void SceneRenderer::SkyboxPass()
    {
        m_RenderGraph.AddPass(
            {.Name = "SkyboxPass",
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 SkyboxDrawCommand cmd;
                 cmd.pipeline = m_SkyboxPipeline;
                 cmd.vao = m_cubeVA;
                 cmd.cubemap = m_skybox.get();
                 cmd.view = m_sceneData.sceneCamera.view;
                 cmd.projection = m_sceneData.sceneCamera.camera.getProjection();

                 Renderer3D::recordSkyboxPass(commandBuffer, cmd);
             }});
    }

    void SceneRenderer::FlushDrawList()
    {
        m_RenderGraph.Reset();

        if (m_sceneData.enableShadows)
            ShadowPass();

        if (m_sceneData.showSkybox)
            SkyboxPass();
        OutlinePass();

        GeometryPass();

        RendererBackend backend(RenderCommand::GetRendererAPI());
        m_RenderGraph.Execute(m_CommandQueue, backend);
        s_MeshDrawList.clear();
    }

    void SceneRenderer::ShadowPass()
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

        m_RenderGraph.AddPass(
            {.Name = "ShadowPass",
             .Execute = [this](CommandBuffer &commandBuffer)
             {
                 commandBuffer.Record([this](RendererAPI &api)
                                      {
                     m_shadowMapFB->bind();
                     RenderCommand::clear();
                     
                     m_ShadowPipeline->bind();
                     auto shader = m_ShadowPipeline->getShader();
                     shader->setMat4("u_LightSpaceMatrix", m_lightSpaceMatrix);
                     
                     for (auto &cmd : s_MeshDrawList) {
                         shader->setMat4("u_Model", cmd.transform);
                         RenderCommand::drawIndexed(cmd.vao, cmd.indexCount, cmd.indexOffset);
                     }
                     
                     if (m_targetFramebuffer) {
                         m_targetFramebuffer->bind();
                     } else {
                         m_shadowMapFB->unbind();  
                     } });
             }});
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
} // namespace Fermion
