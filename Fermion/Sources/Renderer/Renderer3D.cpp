#include "Renderer3D.hpp"
#include "fmpch.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"
#include "glad/glad.h"

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

        std::shared_ptr<Shader> MeshShader;

        std::shared_ptr<Shader> OutlineShader;
        float OutlineWidth = 0.05f;
        float Epsilon = 0.15f;
        glm::vec4 OutlineColor = {1.0f, 0.0f, 0.0f, 0.8f};

        glm::mat4 ViewProjection;
    };

    static Renderer3DData s_Data;
    void Renderer3D::Init(const RendererConfig &config)
    {
        s_Data.MeshShader = Shader::create(config.ShaderPath + "Mesh.glsl");
        s_Data.OutlineShader = Shader::create(config.ShaderPath + "Outline.glsl");
    }

    void Renderer3D::Shutdown()
    {
    }

    void Renderer3D::SetCamera(const Camera &camera, const glm::mat4 &view)
    {
        s_Data.ViewProjection = camera.getProjection() * view;

        s_Data.MeshShader->bind();
        s_Data.MeshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.OutlineShader->bind();
        s_Data.OutlineShader->setMat4("u_View", view);
        s_Data.OutlineShader->setMat4("u_Projection", camera.getProjection());

       
    }
    void Renderer3D::SetCamera(const EditorCamera &camera)
    {
        s_Data.ViewProjection = camera.getViewProjection();

        s_Data.MeshShader->bind();
        s_Data.MeshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.OutlineShader->bind();
        s_Data.OutlineShader->setMat4("u_View", camera.getViewMatrix());
        s_Data.OutlineShader->setMat4("u_Projection", camera.getProjection());

    }
    void Renderer3D::DrawMesh(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID)
    {
        s_Data.MeshShader->bind();
        auto &submeshes = mesh->getSubMeshes();
        auto &materials = mesh->getMaterials();
        auto va = mesh->getVertexArray();
        va->bind();

        s_Data.MeshShader->setMat4("u_Model", transform);
        s_Data.MeshShader->setInt("u_ObjectID", objectID);

        for (auto &submesh : submeshes)
        {
            // 绑定材质
            if (submesh.MaterialIndex < materials.size())
            {
                auto material = materials[submesh.MaterialIndex];
                material->bind(s_Data.MeshShader);
            }

            // 绘制
            RenderCommand::drawIndexed(
                va,
                submesh.IndexCount,
                submesh.IndexOffset);
        }
    }

    void Renderer3D::DrawMesh(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Material> &material, const glm::mat4 &transform, int objectID)
    {
        s_Data.MeshShader->bind();
        auto &subMeshs = mesh->getSubMeshes();

        auto va = mesh->getVertexArray();
        va->bind();

        s_Data.MeshShader->setMat4("u_Model", transform);
        s_Data.MeshShader->setInt("u_ObjectID", objectID);

        for (auto &submesh : subMeshs)
        {
            material->bind(s_Data.MeshShader);
            RenderCommand::drawIndexed(va, submesh.IndexCount, submesh.IndexOffset);
        }
    }

    void Renderer3D::DrawMeshOutline(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID)
    {

        auto &submeshes = mesh->getSubMeshes();
        auto va = mesh->getVertexArray();
        va->bind();

        s_Data.OutlineShader->bind();
        s_Data.OutlineShader->setMat4("u_Model", transform);
        s_Data.OutlineShader->setFloat("u_OutlineWidth", s_Data.OutlineWidth);
        s_Data.OutlineShader->setFloat("u_Epsilon", s_Data.Epsilon);
        s_Data.OutlineShader->setFloat4("u_OutlineColor", s_Data.OutlineColor);
        s_Data.OutlineShader->setInt("u_ObjectID", -1);

        // TODO(Yang): 移动到渲染器后端实现中
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        for (auto &submesh : submeshes)
        {
            RenderCommand::drawIndexed(va, submesh.IndexCount, submesh.IndexOffset);
        }
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);
        glDepthFunc(GL_LESS);
    }
}
