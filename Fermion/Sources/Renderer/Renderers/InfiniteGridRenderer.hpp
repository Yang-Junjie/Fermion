#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Renderer/RenderGraphLegacy.hpp"

namespace Fermion
{
    class VertexArray;
    class Pipeline;
    class Shader;
    struct RenderContext;

    // Grid plane orientation
    enum class GridPlane : int
    {
        XZ = 0,  // Horizontal plane (Y up) - default
        XY = 1,  // Vertical plane (Z forward)
        YZ = 2   // Vertical plane (X right)
    };

    class InfiniteGridRenderer
    {
    public:
        struct Settings
        {
            bool enabled = true;
            GridPlane plane = GridPlane::XZ;
            float gridScale = 3.0f;
            float fadeDistance = 500.0f;
            glm::vec4 gridColorThin = glm::vec4(0.5f, 0.5f, 0.5f, 0.4f);
            glm::vec4 gridColorThick = glm::vec4(0.5f, 0.5f, 0.5f, 0.6f);
            glm::vec4 axisColorX = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
            glm::vec4 axisColorZ = glm::vec4(0.2f, 0.2f, 0.9f, 1.0f);
        };

        InfiniteGridRenderer();
        ~InfiniteGridRenderer() = default;

        void addPass(RenderGraphLegacy& renderGraph,
                     const RenderContext& context,
                     const Settings& settings,
                     ResourceHandle colorTarget,
                     ResourceHandle depthTarget);

        void render(RenderCommandQueue& queue, const RenderContext& context, const Settings& settings);

    private:
        void initializeResources();

    private:
        std::shared_ptr<VertexArray> m_quadVA;
        std::shared_ptr<Pipeline> m_gridPipeline;
        bool m_initialized = false;
    };

} // namespace Fermion
