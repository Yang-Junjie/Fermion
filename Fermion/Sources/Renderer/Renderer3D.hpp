#pragma once
#include "Renderer/Camera.hpp"
#include "Renderer/EditorCamera.hpp"
#include "Renderer/RendererConfig.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"

namespace Fermion
{
    class Renderer3D
    {
    public:
        static void Init(const RendererConfig &config);
        static void Shutdown();

        static void BeginScene(const Camera &camera, const glm::mat4 &view);
        static void BeginScene(const EditorCamera &camera);
        static void EndScene();
        static void Flush();
        static void FlushAndReset();

        static void DrawCube(const glm::mat4 &transform, const glm::vec4 &color, int objectID = -1);
        static void DrawMesh(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID = -1);
        static void DrawMesh(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Material> &material, const glm::mat4 &transform, int objectID = -1);
        static void DrawMeshOutline(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID = -1);
    };

}