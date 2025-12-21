#pragma once
#include "Renderer3D.hpp"
#include "fmpch.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Pipeline.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace Fermion
{
    struct CubeVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec4 color;
        int objectID; // Editor picking
    };

    struct Renderer3DData
    {
        std::shared_ptr<Pipeline> MeshPipeline;
        std::shared_ptr<Pipeline> OutlinePipeline;
        std::shared_ptr<Pipeline> SkyboxPipeline;

        float OutlineWidth = 0.05f;
        float Epsilon = 0.15f;
        glm::vec4 OutlineColor = {1.0f, 0.0f, 0.0f, 0.8f};

        glm::mat4 ViewProjection;
        glm::vec3 CameraPosition;
        std::shared_ptr<VertexArray> cubeVA;

        const uint32_t maxLights = 16;

        EnvironmentLight EnvLight;
    };

    static Renderer3DData s_Data;

    void Renderer3D::Init(const RendererConfig &config)
    {
        // Mesh Pipeline
        {
            PipelineSpecification meshSpec;
            meshSpec.Shader = Shader::create(config.ShaderPath + "Mesh.glsl");
            meshSpec.DepthTest = true;
            meshSpec.DepthWrite = true;
            meshSpec.DepthOperator = DepthCompareOperator::Less;
            meshSpec.Cull = CullMode::Back;

            s_Data.MeshPipeline = Pipeline::Create(meshSpec);
        }

        // Outline Pipeline
        {
            PipelineSpecification outlineSpec;
            outlineSpec.Shader = Shader::create(config.ShaderPath + "Outline.glsl");
            outlineSpec.DepthTest = true;
            outlineSpec.DepthWrite = false;
            outlineSpec.DepthOperator = DepthCompareOperator::LessOrEqual;
            outlineSpec.Cull = CullMode::Front;

            s_Data.OutlinePipeline = Pipeline::Create(outlineSpec);
        }

        // Skybox Pipeline
        {
            PipelineSpecification skyboxSpec;
            skyboxSpec.Shader = Shader::create(config.ShaderPath + "Skybox.glsl");
            skyboxSpec.DepthTest = true;
            skyboxSpec.DepthWrite = false;
            skyboxSpec.DepthOperator = DepthCompareOperator::LessOrEqual;
            skyboxSpec.Cull = CullMode::None;

            s_Data.SkyboxPipeline = Pipeline::Create(skyboxSpec);
        }

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

        s_Data.cubeVA = VertexArray::create();
        s_Data.cubeVA->addVertexBuffer(vertexBuffer);
        s_Data.cubeVA->setIndexBuffer(indexBuffer);
    }

    void Renderer3D::Shutdown()
    {
    }

    std::vector<const PointLight *> SelectTopLightsForMesh(
        const glm::vec3 &meshPosition,
        const glm::vec3 &meshNormal)
    {
        const auto &lights = s_Data.EnvLight.pointLights;
        struct ScoredLight
        {
            const PointLight *light;
            float score;
        };

        std::vector<ScoredLight> candidates;
        candidates.reserve(lights.size());

        for (const auto &light : lights)
        {
            glm::vec3 dir = light.position - meshPosition;
            float dist = glm::length(dir);


            float score = light.intensity / (dist * dist + 1.0f);

            score *= std::max(glm::dot(glm::normalize(dir), meshNormal), 0.0f);

            candidates.push_back({&light, score});
        }

        uint32_t count = std::min(s_Data.maxLights, (uint32_t)candidates.size());
        std::partial_sort(
            candidates.begin(),
            candidates.begin() + count,
            candidates.end(),
            [](const ScoredLight &a, const ScoredLight &b)
            {
                return a.score > b.score;
            });

        std::vector<const PointLight *> topLights;
        topLights.reserve(count);
        for (uint32_t i = 0; i < count; i++)
            topLights.push_back(candidates[i].light);

        return topLights;
    }

    void Renderer3D::SetCamera(const Camera &camera,
                               const glm::mat4 &view,
                               const EnvironmentLight &envLight)
    {
        s_Data.ViewProjection = camera.getProjection() * view;
        s_Data.CameraPosition = glm::vec3(glm::inverse(view)[3]);
        s_Data.EnvLight = envLight; 

        s_Data.MeshPipeline->Bind();
        auto meshShader = s_Data.MeshPipeline->GetShader();
        meshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.OutlinePipeline->Bind();
        auto outlineShader = s_Data.OutlinePipeline->GetShader();
        outlineShader->setMat4("u_View", view);
        outlineShader->setMat4("u_Projection", camera.getProjection());
    }

    void Renderer3D::SetCamera(const EditorCamera &camera,
                               const EnvironmentLight &envLight)
    {
        s_Data.ViewProjection = camera.getViewProjection();
        s_Data.CameraPosition = camera.getPosition();
        s_Data.EnvLight = envLight;

        s_Data.MeshPipeline->Bind();
        auto meshShader = s_Data.MeshPipeline->GetShader();
        meshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.OutlinePipeline->Bind();
        auto outlineShader = s_Data.OutlinePipeline->GetShader();
        outlineShader->setMat4("u_View", camera.getViewMatrix());
        outlineShader->setMat4("u_Projection", camera.getProjection());
    }

    void Renderer3D::DrawMesh(const std::shared_ptr<Mesh> &mesh,
                              const glm::mat4 &transform,
                              int objectID)
    {
        s_Data.MeshPipeline->Bind();
        auto meshShader = s_Data.MeshPipeline->GetShader();
        meshShader->setMat4("u_Model", transform);
        meshShader->setInt("u_ObjectID", objectID);

        glm::vec3 meshPosition = glm::vec3(transform[3]);
        glm::vec3 meshNormal = glm::vec3(0.0f, 1.0f, 0.0f);// TODO: Get from mesh

        auto topLights = SelectTopLightsForMesh(meshPosition, meshNormal);

        meshShader->setInt("u_PointLightCount", topLights.size());
        for (uint32_t i = 0; i < topLights.size(); i++)
        {
            const auto &l = *topLights[i];
            std::string base = "u_PointLights[" + std::to_string(i) + "]";
            meshShader->setFloat3(base + ".position", l.position);
            meshShader->setFloat3(base + ".color", l.color);
            meshShader->setFloat(base + ".intensity", l.intensity);
            meshShader->setFloat(base + ".range", l.range);
        }

        auto &submeshes = mesh->getSubMeshes();
        auto &materials = mesh->getMaterials();
        for (auto &submesh : submeshes)
        {
            if (submesh.MaterialIndex < materials.size())
            {
                auto material = materials[submesh.MaterialIndex];
                material->bind(meshShader);
            }
            RenderCommand::drawIndexed(mesh->getVertexArray(),
                                       submesh.IndexCount,
                                       submesh.IndexOffset);
        }
    }

    void Renderer3D::DrawMesh(const std::shared_ptr<Mesh> &mesh,
                              const std::shared_ptr<Material> &material,
                              const glm::mat4 &transform,
                              int objectID)
    {
        s_Data.MeshPipeline->Bind();
        auto meshShader = s_Data.MeshPipeline->GetShader();
        meshShader->setMat4("u_Model", transform);
        meshShader->setInt("u_ObjectID", objectID);

        glm::vec3 meshPosition = glm::vec3(transform[3]);
        glm::vec3 meshNormal = glm::vec3(0.0f, 1.0f, 0.0f);// TODO: Get from mesh

        auto topLights = SelectTopLightsForMesh(meshPosition, meshNormal);

        meshShader->setInt("u_PointLightCount", topLights.size());
        for (uint32_t i = 0; i < topLights.size(); i++)
        {
            const auto &l = *topLights[i];
            std::string base = "u_PointLights[" + std::to_string(i) + "]";
            meshShader->setFloat3(base + ".position", l.position);
            meshShader->setFloat3(base + ".color", l.color);
            meshShader->setFloat(base + ".intensity", l.intensity);
            meshShader->setFloat(base + ".range", l.range);
        }

        auto &submeshes = mesh->getSubMeshes();
        for (auto &submesh : submeshes)
        {
            material->bind(meshShader);
            RenderCommand::drawIndexed(mesh->getVertexArray(), submesh.IndexCount, submesh.IndexOffset);
        }
    }

    void Renderer3D::DrawMeshOutline(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID)
    {
        s_Data.OutlinePipeline->Bind();
        auto outlineShader = s_Data.OutlinePipeline->GetShader();
        outlineShader->setMat4("u_Model", transform);
        outlineShader->setFloat("u_OutlineWidth", s_Data.OutlineWidth);
        outlineShader->setFloat("u_Epsilon", s_Data.Epsilon);
        outlineShader->setFloat4("u_OutlineColor", s_Data.OutlineColor);
        outlineShader->setInt("u_ObjectID", -1);

        auto &submeshes = mesh->getSubMeshes();
        for (auto &submesh : submeshes)
        {
            RenderCommand::drawIndexed(mesh->getVertexArray(), submesh.IndexCount, submesh.IndexOffset);
        }
    }

    void Renderer3D::DrawSkybox(const std::shared_ptr<TextureCube> &cubemap,
                                const glm::mat4 &view,
                                const glm::mat4 &projection)
    {
        s_Data.SkyboxPipeline->Bind();
        auto skyBoxShader = s_Data.SkyboxPipeline->GetShader();
        skyBoxShader->bind();
        skyBoxShader->setMat4("u_View", glm::mat4(glm::mat3(view)));
        skyBoxShader->setMat4("u_Projection", projection);

        cubemap->bind(0);
        skyBoxShader->setInt("u_Cubemap", 0);

        RenderCommand::drawIndexed(s_Data.cubeVA, 36);
    }

}
