#include "Renderer3D.hpp"
#include "fmpch.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/RenderCommand.hpp"

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
        static const uint32_t MaxCubes = 1000;
        static const uint32_t MaxVertices = MaxCubes * 24;
        static const uint32_t MaxIndices = MaxCubes * 36;

        std::shared_ptr<VertexArray> CubeVA;
        std::shared_ptr<VertexBuffer> CubeVB;
        std::shared_ptr<IndexBuffer> CubeIB;
        std::shared_ptr<Shader> CubeShader;

        std::shared_ptr<Shader> MeshShader;

        uint32_t IndexCount = 0;

        CubeVertex *VertexBufferBase = nullptr;
        CubeVertex *VertexBufferPtr = nullptr;

        glm::mat4 ViewProjection;
    };

    static uint32_t *CreateCubeIndices()
    {
        uint32_t *indices = new uint32_t[Renderer3DData::MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < Renderer3DData::MaxIndices; i += 36)
        {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;

            indices[i + 6] = offset + 4;
            indices[i + 7] = offset + 5;
            indices[i + 8] = offset + 6;
            indices[i + 9] = offset + 6;
            indices[i + 10] = offset + 7;
            indices[i + 11] = offset + 4;

            indices[i + 12] = offset + 8;
            indices[i + 13] = offset + 9;
            indices[i + 14] = offset + 10;
            indices[i + 15] = offset + 10;
            indices[i + 16] = offset + 11;
            indices[i + 17] = offset + 8;

            indices[i + 18] = offset + 12;
            indices[i + 19] = offset + 13;
            indices[i + 20] = offset + 14;
            indices[i + 21] = offset + 14;
            indices[i + 22] = offset + 15;
            indices[i + 23] = offset + 12;

            indices[i + 24] = offset + 16;
            indices[i + 25] = offset + 17;
            indices[i + 26] = offset + 18;
            indices[i + 27] = offset + 18;
            indices[i + 28] = offset + 19;
            indices[i + 29] = offset + 16;

            indices[i + 30] = offset + 20;
            indices[i + 31] = offset + 21;
            indices[i + 32] = offset + 22;
            indices[i + 33] = offset + 22;
            indices[i + 34] = offset + 23;
            indices[i + 35] = offset + 20;

            offset += 24;
        }

        return indices;
    }
    static const glm::vec3 s_CubePositions[24] = {
        // Front (+Z)
        {-0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},

        // Back (-Z)
        {0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},

        // Left (-X)
        {-0.5f, -0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f},
        {-0.5f, 0.5f, 0.5f},
        {-0.5f, 0.5f, -0.5f},

        // Right (+X)
        {0.5f, -0.5f, 0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, 0.5f, -0.5f},
        {0.5f, 0.5f, 0.5f},

        // Top (+Y)
        {-0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, -0.5f},
        {-0.5f, 0.5f, -0.5f},

        // Bottom (-Y)
        {-0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, -0.5f},
        {0.5f, -0.5f, 0.5f},
        {-0.5f, -0.5f, 0.5f}};
    static const glm::vec3 s_CubeNormals[24] = {
        // Front
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f},

        // Back
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f},

        // Left
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f},

        // Right
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},

        // Top
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},

        // Bottom
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}};

    static Renderer3DData s_Data;
    void Renderer3D::Init(const RendererConfig &config)
    {
        s_Data.CubeVA = VertexArray::create();

        s_Data.CubeVB = VertexBuffer::create(
            Renderer3DData::MaxVertices * sizeof(CubeVertex));

        s_Data.CubeVB->setLayout({{ShaderDataType::Float3, "a_Position"},
                                  {ShaderDataType::Float3, "a_Normal"},
                                  {ShaderDataType::Float4, "a_Color"},
                                  {ShaderDataType::Int, "a_ObjectID"}});

        s_Data.CubeVA->addVertexBuffer(s_Data.CubeVB);

        uint32_t *indices = CreateCubeIndices();
        s_Data.CubeIB = IndexBuffer::create(indices, Renderer3DData::MaxIndices);
        s_Data.CubeVA->setIndexBuffer(s_Data.CubeIB);
        delete[] indices;

        s_Data.VertexBufferBase = new CubeVertex[Renderer3DData::MaxVertices];

        s_Data.CubeShader = Shader::create(config.ShaderPath + "Cube.glsl");
        s_Data.MeshShader = Shader::create(config.ShaderPath + "Mesh.glsl");
    }
    void Renderer3D::Flush()
    {
        if (s_Data.IndexCount == 0)
            return;

        uint32_t dataSize =
            (uint32_t)((uint8_t *)s_Data.VertexBufferPtr -
                       (uint8_t *)s_Data.VertexBufferBase);

        s_Data.CubeVB->setData(s_Data.VertexBufferBase, dataSize);

        RenderCommand::drawIndexed(
            s_Data.CubeVA,
            s_Data.IndexCount);
    }
    void Renderer3D::FlushAndReset()
    {
        EndScene();

        s_Data.IndexCount = 0;
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
    }
    void Renderer3D::Shutdown()
    {
        delete[] s_Data.VertexBufferBase;
        s_Data.VertexBufferBase = nullptr;
    }

    void Renderer3D::BeginScene(const Camera &camera, const glm::mat4 &view)
    {
        s_Data.ViewProjection = camera.getProjection() * view;
        s_Data.CubeShader->bind();
        s_Data.CubeShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.MeshShader->bind();
        s_Data.MeshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.IndexCount = 0;
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
    }
    void Renderer3D::BeginScene(const EditorCamera &camera)
    {
        s_Data.ViewProjection = camera.getViewProjection();
        s_Data.CubeShader->bind();
        s_Data.CubeShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.MeshShader->bind();
        s_Data.MeshShader->setMat4("u_ViewProjection", s_Data.ViewProjection);

        s_Data.IndexCount = 0;
        s_Data.VertexBufferPtr = s_Data.VertexBufferBase;
    }
    void Renderer3D::EndScene()
    {
        Flush();
    }

    void Renderer3D::DrawCube(
        const glm::mat4 &transform,
        const glm::vec4 &color,
        int objectID)
    {
        if (s_Data.IndexCount >= Renderer3DData::MaxIndices)
            FlushAndReset();

        for (uint32_t i = 0; i < 24; i++)
        {
            s_Data.VertexBufferPtr->position =
                transform * glm::vec4(s_CubePositions[i], 1.0f);
            s_Data.VertexBufferPtr->normal =
                glm::mat3(transform) * s_CubeNormals[i];
            s_Data.VertexBufferPtr->color = color;
            s_Data.VertexBufferPtr->objectID = objectID;
            s_Data.VertexBufferPtr++;
        }

        s_Data.IndexCount += 36;
    }
    void Renderer3D::DrawMesh(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID)
    {
        if (!mesh)
            return;

        auto &submeshes = mesh->getSubMeshes();
        auto &materials = mesh->getMaterials();
        auto va = mesh->getVertexArray();
        va->bind();

        for (auto &submesh : submeshes)
        {
            // 绑定材质
            if (submesh.MaterialIndex < materials.size())
            {
                auto material = materials[submesh.MaterialIndex];
                material->bind(s_Data.MeshShader);
            }

            // 设置模型矩阵
            s_Data.MeshShader->setMat4("u_Model", transform);

            // 绘制
            RenderCommand::drawIndexed(
                va,
                submesh.IndexCount,
                submesh.IndexOffset);
        }
    }

}
