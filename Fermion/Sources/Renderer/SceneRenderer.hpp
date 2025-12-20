#pragma once
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/EditorCamera.hpp"
#include "Renderer/DebugRenderer.hpp"

#include "Renderer/RenderPass.hpp"
#include "Renderer/RenderGraph.hpp"
#include <vector>

namespace Fermion
{
    class DebugRenderer;
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

        struct MeshDrawCommand
        {
            std::shared_ptr<Mesh> mesh;
            std::shared_ptr<Material> material;
            glm::mat4 transform;

            int objectID = -1;
            bool drawOutline = false;
        };

        SceneRenderer();
        void beginScene(const Camera &camera, const glm::mat4 &transform);
        void beginScene(const EditorCamera &camera);
        void beginScene(const SceneRendererCamera &camera);

        void endScene();

        void drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID = -1);
        void drawString(const std::string &string, const glm::mat4 &transform, const TextComponent &component, int objectID = -1);
        void drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness = 1.0f, float fade = 0.005f, int objectID = -1);
        void drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, int objectId = -1);
        void drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId = -1);
        void DrawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color);
        void SetLineWidth(float thickness);

        void SubmitMesh(MeshComponent &meshComponent, glm::mat4 transform, int objectId = -1, bool drawOutline = false);
        void SubmitMesh(MeshComponent &meshComponent, MaterialComponent &materialComponent, glm::mat4 transform, int objectId = -1, bool drawOutline = false);

        void setScene(std::shared_ptr<Scene> scene) { m_scene = scene; }
        std::shared_ptr<Scene> getScene() const { return m_scene; }
        std::shared_ptr<DebugRenderer> GetDebugRenderer() const { return m_debugRenderer; }

        Statistics getStatistics() const;

    private:
        void GeometryPass();
        void OutlinePass();
        void FlushDrawList();
        void SkyboxPass();

    private:
        std::shared_ptr<DebugRenderer> m_debugRenderer;

        std::shared_ptr<Scene> m_scene;
        std::vector<MeshDrawCommand> s_MeshDrawList;
        std::shared_ptr<TextureCube> m_skybox;

        RenderGraph m_RenderGraph;
        struct SceneInfo
        {
            SceneRendererCamera sceneCamera;
            EnvironmentLight sceneEnvironmentLight;
        } m_sceneData;
    };
}
