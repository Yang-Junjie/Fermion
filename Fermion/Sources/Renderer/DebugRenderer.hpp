#pragma once
#include "SceneRenderer.hpp"


namespace Fermion {
    class DebugRenderer {
    public:
        using RenderQueue = std::vector<std::function<void(std::shared_ptr<SceneRenderer>)> >;

    public:
        DebugRenderer() = default;

        ~DebugRenderer() = default;

        void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color = glm::vec4(1.0f));

        void drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                               const glm::vec4 &color = glm::vec4(1.0f));

        void SetLineWidth(float thickness);

        RenderQueue &GetRenderQueue() {
            return m_renderQueue;
        }

        void ClearRenderQueue() {
            m_renderQueue.clear();
        }

    private:
        RenderQueue m_renderQueue;
    };
} // namespace Fermion
