#include "DebugRenderer.hpp"

namespace Fermion
{
    void DebugRenderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color)
    {
        m_renderQueue.emplace_back([p0, p1, color](std::shared_ptr<SceneRenderer> renderer)
                                   { renderer->DrawLine(p0, p1, color); });
    }
    void DebugRenderer::SetLineWidth(float thickness)
    {
        m_renderQueue.emplace_back([thickness](std::shared_ptr<SceneRenderer> renderer)
                                   { renderer->SetLineWidth(thickness); });
    }
}