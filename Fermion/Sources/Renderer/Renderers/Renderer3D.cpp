#pragma once
#include "Renderer3D.hpp"
#include "fmpch.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Pipeline.hpp"
#include "Renderer.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace Fermion
{

    struct Renderer3DData
    {
        std::shared_ptr<Pipeline> meshPipeline;
        std::shared_ptr<Pipeline> pbrMeshPipeline;

        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;

        const uint32_t maxLights = 16;

        EnvironmentLight EnvLight;
    };

    static Renderer3DData s_Data;

    void Renderer3D::init(const RendererConfig &config)
    {
        // Mesh Pipeline (传统Phong)
        {
            PipelineSpecification meshSpec;
            meshSpec.shader = Renderer::getShaderLibrary()->get("Mesh");
            meshSpec.depthTest = true;
            meshSpec.depthWrite = true;
            meshSpec.depthOperator = DepthCompareOperator::Less;
            meshSpec.cull = CullMode::Back;

            s_Data.meshPipeline = Pipeline::create(meshSpec);
        }

        // PBR Mesh Pipeline
        {
            PipelineSpecification pbrSpec;
            pbrSpec.shader = Renderer::getShaderLibrary()->get("PBRMesh");
            pbrSpec.depthTest = true;
            pbrSpec.depthWrite = true;
            pbrSpec.depthOperator = DepthCompareOperator::Less;
            pbrSpec.cull = CullMode::Back;

            s_Data.pbrMeshPipeline = Pipeline::create(pbrSpec);
        }
    }

    void Renderer3D::shutdown()
    {
    }

    // TODO(Yang):去除Pipeline在Rendere3d
    void Renderer3D::updateViewState(const Camera &camera,
                                     const glm::mat4 &view,
                                     const EnvironmentLight &envLight)
    {
        s_Data.ViewProjection = camera.getProjection() * view;
        s_Data.CameraPosition = glm::vec3(glm::inverse(view)[3]);
        s_Data.EnvLight = envLight;

        s_Data.meshPipeline->bind();
        auto meshShader = Renderer::getShaderLibrary()->get("Mesh");
        meshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.pbrMeshPipeline->bind();
        auto pbrShader = Renderer::getShaderLibrary()->get("PBRMesh");
        pbrShader->setMat4("u_ViewProjection", s_Data.ViewProjection);
        pbrShader->setFloat3("u_CameraPosition", s_Data.CameraPosition);
    }

    void Renderer3D::updateViewState(const EditorCamera &camera,
                                     const EnvironmentLight &envLight)
    {
        s_Data.ViewProjection = camera.getViewProjection();
        s_Data.CameraPosition = camera.getPosition();
        s_Data.EnvLight = envLight;

        s_Data.meshPipeline->bind();
        auto meshShader = Renderer::getShaderLibrary()->get("Mesh");
        meshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.pbrMeshPipeline->bind();
        auto pbrShader = Renderer::getShaderLibrary()->get("PBRMesh");
        pbrShader->setMat4("u_ViewProjection", s_Data.ViewProjection);
        pbrShader->setFloat3("u_CameraPosition", s_Data.CameraPosition);
    }

    void Renderer3D::recordGeometryPass(CommandBuffer &commandBuffer,
                                        const std::vector<MeshDrawCommand> &drawCommands)
    {
        commandBuffer.record([&drawCommands](RendererAPI &api)
                             {
                                 std::shared_ptr<Pipeline> currentPipeline = nullptr;

                                 for (auto &cmd : drawCommands)
                                 {
                                     if (!cmd.visible)
                                         continue;
                                     if (currentPipeline != cmd.pipeline)
                                     {
                                         currentPipeline = cmd.pipeline;
                                         currentPipeline->bind();
                                     }

                                     auto shader = currentPipeline->getShader();
                                     shader->setMat4("u_Model", cmd.transform);
                                     shader->setInt("u_ObjectID", cmd.objectID);

                                     const auto &dirLight = s_Data.EnvLight.directionalLight;
                                     shader->setFloat3("u_DirectionalLight.direction", -dirLight.direction);
                                     shader->setFloat3("u_DirectionalLight.color", dirLight.color);
                                     shader->setFloat("u_DirectionalLight.intensity", dirLight.intensity);

                                     // PointLight
                                     uint32_t pointCount = std::min(s_Data.maxLights, (uint32_t)s_Data.EnvLight.pointLights.size());
                                     shader->setInt("u_PointLightCount", pointCount);
                                     for (uint32_t i = 0; i < pointCount; i++)
                                     {
                                         const auto &l = s_Data.EnvLight.pointLights[i];
                                         std::string base = "u_PointLights[" + std::to_string(i) + "]";
                                         shader->setFloat3(base + ".position", l.position);
                                         shader->setFloat3(base + ".color", l.color);
                                         shader->setFloat(base + ".intensity", l.intensity);
                                         shader->setFloat(base + ".range", l.range);
                                     }

                                     // SpotLight
                                     uint32_t spotCount = std::min(s_Data.maxLights, (uint32_t)s_Data.EnvLight.spotLights.size());
                                     shader->setInt("u_SpotLightCount", spotCount);
                                     for (uint32_t i = 0; i < spotCount; i++)
                                     {
                                         const auto &l = s_Data.EnvLight.spotLights[i];
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
                                     RenderCommand::drawIndexed(cmd.vao,
                                                                cmd.indexCount,
                                                                cmd.indexOffset);
                                 } });
    }
    void Renderer3D::recordSkyboxPass(CommandBuffer &commandBuffer, const SkyboxDrawCommand &drawCommand)
    {
        commandBuffer.record([drawCommand](RendererAPI &)
                             {
                if (!drawCommand.pipeline || !drawCommand.vao || !drawCommand.cubemap) {
                    return;
                }

                drawCommand.pipeline->bind();
                auto shader = drawCommand.pipeline->getShader();
                shader->bind();
                shader->setMat4("u_View", glm::mat4(glm::mat3(drawCommand.view)));
                shader->setMat4("u_Projection", drawCommand.projection);

                drawCommand.cubemap->bind(0);
                shader->setInt("u_Cubemap", 0);

                RenderCommand::drawIndexed(drawCommand.vao, 36); });
    }

} // namespace Fermion
