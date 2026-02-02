#pragma once
#include "fmpch.hpp"
#include <glm/glm.hpp>

namespace Fermion
{
    class SceneRenderer;
    class EditorCamera;

    class DebugRenderer
    {
    public:
        using RenderQueue = std::vector<std::function<void(std::shared_ptr<SceneRenderer>)>>;

    public:
        DebugRenderer() = default;

        ~DebugRenderer() = default;

        void DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color = glm::vec4(1.0f));

        void drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                               const glm::vec4 &color = glm::vec4(1.0f));
        void drawInfiniteLine(const glm::vec3 &point, const glm::vec3 &direction, const glm::vec4 &color, const EditorCamera &camera);

        void SetLineWidth(float thickness);

        RenderQueue &GetRenderQueue()
        {
            return m_renderQueue;
        }

        void ClearRenderQueue()
        {
            m_renderQueue.clear();
        }

    private:
        RenderQueue m_renderQueue;
    };
} // namespace Fermion
