#include "SceneRenderer.hpp"
#include "Renderer2D.hpp"

namespace Fermion
{
    void SceneRenderer::beginScene(const Camera &camera, const glm::mat4 &transform)
    {
        SceneRendererCamera camInfo{};
        camInfo.camera = camera;
        camInfo.transform = transform;
        camInfo.near = 0.0f;
        camInfo.far = 0.0f;
        camInfo.fov = 0.0f;
        m_sceneData.sceneCamera = camInfo;

        Renderer2D::beginScene(camera, transform);
    }

    void SceneRenderer::beginScene(const EditorCamera &camera)
    {
        SceneRendererCamera camInfo{};
        camInfo.camera = camera;
        camInfo.transform = glm::inverse(camera.getViewMatrix());
        camInfo.near = 0.0f;
        camInfo.far = 0.0f;
        camInfo.fov = 0.0f;
        m_sceneData.sceneCamera = camInfo;
        Renderer2D::beginScene(camera);
    }

    void SceneRenderer::beginScene(const SceneRendererCamera &camera)
    {
        m_sceneData.sceneCamera = camera;
        Renderer2D::beginScene(camera.camera, camera.transform);
    }

    void SceneRenderer::endScene()
    {
        Renderer2D::endScene();
    }

    void SceneRenderer::drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID)
    {
        if (sprite.texture)
            Renderer2D::drawQuad(transform, sprite.texture,
                                 sprite.tilingFactor, sprite.color, objectID);
        else
            Renderer2D::drawQuad(transform, sprite.color, objectID);
    }

    void SceneRenderer::drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness, float fade, int objectID)
    {
        Renderer2D::drawCircle(transform, color, thickness, fade, objectID);
    }
    void SceneRenderer::drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, int objectId)
    {
        Renderer2D::drawRect(position, size, color, objectId);
    }
    void SceneRenderer::drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId)
    {
        Renderer2D::drawRect(transform, color, objectId);
    }

    SceneRenderer::Statistics SceneRenderer::getStatistics() const
    {
        Renderer2D::Satistics stats2D = Renderer2D::getStatistics();
        Statistics result;
        result.drawCalls = stats2D.drawCalls;
        result.quadCount = stats2D.quadCount;
        result.lineCount = stats2D.lineCount;
        result.circleCount = stats2D.circleCount;
        return result;
    }
}
