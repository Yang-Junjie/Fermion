#pragma once
#include "Renderer3D.hpp"
#include "fmpch.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer/Renderer.hpp"


#include "glm/gtc/matrix_transform.hpp"

namespace Fermion {

    struct Renderer3DData {
        std::shared_ptr<Pipeline> meshPipeline;

        std::shared_ptr<Pipeline> skyboxPipeline;
        

        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;
        std::shared_ptr<VertexArray> cubeVA;

        const uint32_t maxLights = 16;

        EnvironmentLight EnvLight;
    };

    static Renderer3DData s_Data;

    void Renderer3D::init(const RendererConfig &config) {
        // Mesh Pipeline
        {
            PipelineSpecification meshSpec;
            meshSpec.shader = Renderer::getShaderLibrary()->get("Mesh");
            meshSpec.depthTest = true;
            meshSpec.depthWrite = true;
            meshSpec.depthOperator = DepthCompareOperator::Less;
            meshSpec.cull = CullMode::Back;

            s_Data.meshPipeline = Pipeline::create(meshSpec);
        }

        // Skybox Pipeline
        {
            PipelineSpecification skyboxSpec;
            skyboxSpec.shader = Renderer::getShaderLibrary()->get("Skybox");
            skyboxSpec.depthTest = true;
            skyboxSpec.depthWrite = false;
            skyboxSpec.depthOperator = DepthCompareOperator::LessOrEqual;
            skyboxSpec.cull = CullMode::None;

            s_Data.skyboxPipeline = Pipeline::create(skyboxSpec);
        }

        float skyboxVertices[] = {
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f
        };

        uint32_t skyboxIndices[] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            4, 5, 1, 1, 0, 4,
            3, 2, 6, 6, 7, 3,
            4, 0, 3, 3, 7, 4,
            1, 5, 6, 6, 2, 1
        };

        auto vertexBuffer = VertexBuffer::create(skyboxVertices, sizeof(skyboxVertices));
        vertexBuffer->setLayout({{ShaderDataType::Float3, "a_Position"}});

        auto indexBuffer = IndexBuffer::create(skyboxIndices, sizeof(skyboxIndices) / sizeof(uint32_t));

        s_Data.cubeVA = VertexArray::create();
        s_Data.cubeVA->addVertexBuffer(vertexBuffer);
        s_Data.cubeVA->setIndexBuffer(indexBuffer);
    }

    void Renderer3D::shutdown() {
    }

    void Renderer3D::updateViewState(const Camera &camera,
                               const glm::mat4 &view,
                               const EnvironmentLight &envLight) {
        s_Data.ViewProjection = camera.getProjection() * view;
        s_Data.CameraPosition = glm::vec3(glm::inverse(view)[3]);
        s_Data.EnvLight = envLight;

        s_Data.meshPipeline->bind();
        auto meshShader = s_Data.meshPipeline->getShader();
        meshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

    }

    void Renderer3D::updateViewState(const EditorCamera &camera,
                               const EnvironmentLight &envLight) {
        s_Data.ViewProjection = camera.getViewProjection();
        s_Data.CameraPosition = camera.getPosition();
        s_Data.EnvLight = envLight;

        s_Data.meshPipeline->bind();
        auto meshShader = s_Data.meshPipeline->getShader();
        meshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

    }

    void Renderer3D::drawMesh(const std::shared_ptr<Mesh> &mesh,
                              const glm::mat4 &transform,
                              int objectID) {
        s_Data.meshPipeline->bind();
        auto meshShader = s_Data.meshPipeline->getShader();
        meshShader->setMat4("u_Model", transform);
        meshShader->setInt("u_ObjectID", objectID);

        {
            const auto &dirLight = s_Data.EnvLight.directionalLight;

            const glm::vec3 dir = -dirLight.direction; // 注意：光照方向指向场景
            meshShader->setFloat3("u_DirectionalLight.direction", dir);
            meshShader->setFloat3("u_DirectionalLight.color", dirLight.color);
            meshShader->setFloat("u_DirectionalLight.intensity", dirLight.intensity);
        }

        const auto &pointLights = s_Data.EnvLight.pointLights;
        uint32_t pointCount = std::min(s_Data.maxLights, (uint32_t) pointLights.size());
        meshShader->setInt("u_PointLightCount", pointCount);

        for (uint32_t i = 0; i < pointCount; i++) {
            const auto &l = pointLights[i];
            std::string base = "u_PointLights[" + std::to_string(i) + "]";

            meshShader->setFloat3(base + ".position", l.position);
            meshShader->setFloat3(base + ".color", l.color);
            meshShader->setFloat(base + ".intensity", l.intensity);
            meshShader->setFloat(base + ".range", l.range);
        }

        const auto &spotLights = s_Data.EnvLight.spotLights;
        uint32_t spotCount = std::min(s_Data.maxLights, (uint32_t) spotLights.size());

        meshShader->setInt("u_SpotLightCount", spotCount);

        for (uint32_t i = 0; i < spotCount; i++) {
            const auto &l = spotLights[i];
            std::string base = "u_SpotLights[" + std::to_string(i) + "]";

            meshShader->setFloat3(base + ".position", l.position);
            meshShader->setFloat3(base + ".direction", glm::normalize(l.direction));

            meshShader->setFloat3(base + ".color", l.color);
            meshShader->setFloat(base + ".intensity", l.intensity);

            meshShader->setFloat(base + ".range", l.range);
            meshShader->setFloat(base + ".innerConeAngle", l.innerConeAngle);
            meshShader->setFloat(base + ".outerConeAngle", l.outerConeAngle);
        }

        auto &submeshes = mesh->getSubMeshes();
        auto &materials = mesh->getMaterials();
        for (auto &submesh: submeshes) {
            if (submesh.MaterialIndex < materials.size()) {
                auto material = materials[submesh.MaterialIndex];
                material->bind(meshShader);
            }
            RenderCommand::drawIndexed(mesh->getVertexArray(),
                                       submesh.IndexCount,
                                       submesh.IndexOffset);
        }
    }

    void Renderer3D::drawMesh(const std::shared_ptr<Mesh> &mesh,
                              const std::shared_ptr<Material> &material,
                              const glm::mat4 &transform,
                              int objectID) {
        s_Data.meshPipeline->bind();
        auto meshShader = s_Data.meshPipeline->getShader();
        meshShader->setMat4("u_Model", transform);
        meshShader->setInt("u_ObjectID", objectID);

        {
            const auto &dirLight = s_Data.EnvLight.directionalLight;

            glm::vec3 dir = -dirLight.direction; // 注意：光照方向指向场景
            meshShader->setFloat3("u_DirectionalLight.direction", dir);
            meshShader->setFloat3("u_DirectionalLight.color", dirLight.color);
            meshShader->setFloat("u_DirectionalLight.intensity", dirLight.intensity);
        }

        const auto &pointLights = s_Data.EnvLight.pointLights;
        uint32_t pointCount = std::min(s_Data.maxLights, (uint32_t) pointLights.size());

        meshShader->setInt("u_PointLightCount", pointCount);

        for (uint32_t i = 0; i < pointCount; i++) {
            const auto &l = pointLights[i];
            std::string base = "u_PointLights[" + std::to_string(i) + "]";

            meshShader->setFloat3(base + ".position", l.position);
            meshShader->setFloat3(base + ".color", l.color);
            meshShader->setFloat(base + ".intensity", l.intensity);
            meshShader->setFloat(base + ".range", l.range);
        }

        const auto &spotLights = s_Data.EnvLight.spotLights;
        uint32_t spotCount = std::min(s_Data.maxLights, (uint32_t) spotLights.size());

        meshShader->setInt("u_SpotLightCount", spotCount);

        for (uint32_t i = 0; i < spotCount; i++) {
            const auto &l = spotLights[i];
            std::string base = "u_SpotLights[" + std::to_string(i) + "]";

            meshShader->setFloat3(base + ".position", l.position);
            meshShader->setFloat3(base + ".direction", glm::normalize(l.direction));

            meshShader->setFloat3(base + ".color", l.color);
            meshShader->setFloat(base + ".intensity", l.intensity);

            meshShader->setFloat(base + ".range", l.range);
            meshShader->setFloat(base + ".innerConeAngle", l.innerConeAngle);
            meshShader->setFloat(base + ".outerConeAngle", l.outerConeAngle);
        }

        auto &submeshes = mesh->getSubMeshes();
        for (auto &submesh: submeshes) {
            material->bind(meshShader);
            RenderCommand::drawIndexed(mesh->getVertexArray(), submesh.IndexCount, submesh.IndexOffset);
        }
    }

    void Renderer3D::drawSkybox(const TextureCube *cubemap,
                                const glm::mat4 &view,
                                const glm::mat4 &projection) {
        s_Data.skyboxPipeline->bind();
        auto skyBoxShader = s_Data.skyboxPipeline->getShader();
        skyBoxShader->bind();
        skyBoxShader->setMat4("u_View", glm::mat4(glm::mat3(view)));
        skyBoxShader->setMat4("u_Projection", projection);

        cubemap->bind(0);
        skyBoxShader->setInt("u_Cubemap", 0);

        RenderCommand::drawIndexed(s_Data.cubeVA, 36);
    }


    void Renderer3D::recordGeometryPass(CommandBuffer &commandBuffer,
                                        const std::vector<MeshDrawCommand> &drawCommands) {
        const auto *commands = &drawCommands;
        commandBuffer.Record(
            [commands](RendererAPI &) {
                for (auto &mesh: *commands) {
                    if (mesh.material) {
                        Renderer3D::drawMesh(mesh.mesh, mesh.material, mesh.transform, mesh.objectID);
                    } else {
                        Renderer3D::drawMesh(mesh.mesh, mesh.transform, mesh.objectID);
                    }
                }
            });
    }


    void Renderer3D::recordSkyboxPass(CommandBuffer &commandBuffer, const TextureCube *cubemap,
                                      const glm::mat4 &view, const glm::mat4 &projection) {
        commandBuffer.Record(
            [cubemap, view, projection](RendererAPI &) {
                Renderer3D::drawSkybox(cubemap, view, projection);
            });
    }
} // namespace Fermion
