#include "DebugRenderer.hpp"
#include "SceneRenderer.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "Renderer2D.hpp"
namespace Fermion
{
    void DebugRenderer::DrawLine(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec4 &color)
    {
        m_renderQueue.emplace_back([p0, p1, color](std::shared_ptr<SceneRenderer> renderer)
                                   { renderer->drawLine(p0, p1, color); });
    }

    void DebugRenderer::SetLineWidth(float thickness)
    {
        m_renderQueue.emplace_back([thickness](std::shared_ptr<SceneRenderer> renderer)
                                   { renderer->setLineWidth(thickness); });
    }

    void DebugRenderer::drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size,
                                          const glm::vec4 &color)
    {
        m_renderQueue.emplace_back([translation, size, color](std::shared_ptr<SceneRenderer> renderer)
                                   { renderer->drawQuadBillboard(translation, size, color); });
    }

    void DebugRenderer::drawInfiniteLine(const glm::vec3 &point, const glm::vec3 &direction, const glm::vec4 &color, const EditorCamera &camera)
    {
        m_renderQueue.emplace_back([point, direction, color, camera](std::shared_ptr<SceneRenderer> renderer)
                                   {
                                       float big = camera.getFarCilp() * 2.0f;

                                       glm::vec3 p0 = point - direction * big;
                                       glm::vec3 p1 = point + direction * big;

                                       renderer->drawLine(p0, p1, color); });
    }

} // namespace Fermion
