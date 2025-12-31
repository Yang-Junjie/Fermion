#include "DebugRenderer.hpp"
#include "Renderer2D.hpp"
namespace Fermion {
    void DebugRenderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color) {
        m_renderQueue.emplace_back([p0, p1, color](std::shared_ptr<SceneRenderer> renderer) {
            renderer->drawLine(p0, p1, color);
        });
    }

    void DebugRenderer::SetLineWidth(float thickness) {
        m_renderQueue.emplace_back([thickness](std::shared_ptr<SceneRenderer> renderer) {
            renderer->setLineWidth(thickness);
        });
    }

    void DebugRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                                          const glm::vec4 &color) {
        m_renderQueue.emplace_back([translation, size, color](std::shared_ptr<SceneRenderer> renderer) {
            renderer->drawQuadBillboard(translation, size, color);
        });
    }
} // namespace Fermion
