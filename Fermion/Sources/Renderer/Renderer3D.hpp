#pragma once
#include "Renderer/Camera.hpp"
#include "Renderer/EditorCamera.hpp"
#include "Renderer/RendererConfig.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include "Scene/Scene.hpp"
namespace Fermion
{
    class Renderer3D
    {
    public:
        static void Init(const RendererConfig &config);
        static void Shutdown();

        static void SetCamera(const Camera &camera, const glm::mat4 &view, const EnvironmentLight &light);
        static void SetCamera(const EditorCamera &camera, const EnvironmentLight &light);

        static void DrawMesh(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID = -1);
        static void DrawMesh(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Material> &material, const glm::mat4 &transform, int objectID = -1);
        static void DrawMeshOutline(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID = -1);
        static void DrawSkybox(const std::shared_ptr<TextureCube> &cubemap, const glm::mat4 &view, const glm::mat4 &projection);

    };

}