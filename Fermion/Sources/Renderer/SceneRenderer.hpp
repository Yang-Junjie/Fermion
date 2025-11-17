#pragma once
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/EditorCamera.hpp"

namespace Fermion
{
    class SceneRenderer
    {
    public:
        struct SceneRendererCamera
        {
            Camera camera;
            glm::mat4 view;
        };

        struct Statistics
        {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;
            uint32_t lineCount = 0;
            uint32_t circleCount = 0;

            uint32_t getTotalVertexCount() const
            {
                return quadCount * 4 + lineCount * 2 + circleCount * 4;
            }

            uint32_t getTotalIndexCount() const
            {
                return quadCount * 6 + circleCount * 6;
            }
        };

        void beginScene(const Camera &camera, const glm::mat4 &transform);
        void beginScene(const EditorCamera &camera);
        void beginScene(const SceneRendererCamera &camera);

        void endScene();
        void drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID = -1);
        void drawString(const std::string &string, const glm::mat4 &transform, const TextComponent &component, int objectID = -1);
        void drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness = 1.0f, float fade = 0.005f, int objectID = -1);
        void drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, int objectId = -1);
        void drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId = -1);

        void setScene(std::shared_ptr<Scene> scene) { m_scene = scene; }
        std::shared_ptr<Scene> getScene() const { return m_scene; }

        Statistics getStatistics() const;

    private:
        std::shared_ptr<Scene> m_scene;

        struct SceneInfo
        {
            SceneRendererCamera sceneCamera;
        } m_sceneData;
    };
}
