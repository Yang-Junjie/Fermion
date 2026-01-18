#pragma once
#include <vector>

#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer/RendererConfig.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include "Renderer/CommandBuffer.hpp"
#include "Renderer/Model/Mesh.hpp"
#include "Renderer/Model/Material.hpp"
#include "Scene/Scene.hpp"

namespace Fermion
{
    class Renderer3D
    {
    public:
        static void init(const RendererConfig &config);

        static void shutdown();

        static void updateViewState(const Camera &camera, const glm::mat4 &view, const EnvironmentLight &light);

        static void updateViewState(const EditorCamera &camera, const EnvironmentLight &light);

        static void recordGeometryPass(CommandBuffer &commandBuffer, const std::vector<MeshDrawCommand> &drawCommands);

        static void recordSkyboxPass(CommandBuffer &commandBuffer, const SkyboxDrawCommand &drawCommand);
    };
} // namespace Fermion
