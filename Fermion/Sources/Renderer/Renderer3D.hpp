#pragma once
#include <vector>

#include "Camera/Camera.hpp"
#include "Camera/EditorCamera.hpp"
#include "RendererConfig.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Model/Mesh.hpp"
#include "Model/Material.hpp"
#include "Scene/Scene.hpp"

namespace Fermion {
    class Renderer3D {
    public:
        static void init(const RendererConfig &config);

        static void shutdown();

        static void updateViewState(const Camera &camera, const glm::mat4 &view, const EnvironmentLight &light);

        static void updateViewState(const EditorCamera &camera, const EnvironmentLight &light);

        static void drawMesh(const std::shared_ptr<Mesh> &mesh, const glm::mat4 &transform, int objectID = -1);

        static void drawMesh(const std::shared_ptr<Mesh> &mesh, const std::shared_ptr<Material> &material,
                             const glm::mat4 &transform, int objectID = -1);


        static void drawSkybox(const TextureCube* cubeMap, const glm::mat4 &view,
                               const glm::mat4 &projection);

        static void recordGeometryPass(CommandBuffer &commandBuffer, const std::vector<MeshDrawCommand> &drawCommands);

        

        static void recordSkyboxPass(CommandBuffer &commandBuffer, const TextureCube *cubeMap,
                                     const glm::mat4 &view, const glm::mat4 &projection);


    };
} // namespace Fermion
