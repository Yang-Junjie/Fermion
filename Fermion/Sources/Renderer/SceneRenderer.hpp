#pragma once
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include "Model/Mesh.hpp"
#include "Model/Material.hpp"
#include "Camera/Camera.hpp"
#include "Camera/EditorCamera.hpp"
#include "DebugRenderer.hpp"

#include "RenderPass.hpp"
#include "RenderGraph.hpp"
#include "RenderDrawCommand.hpp"
#include <vector>
#include "RenderCommandQueue.hpp"

namespace Fermion {
    class DebugRenderer;

    class SceneRenderer {
    public:
        struct SceneRendererCamera {
            Camera camera;
            glm::mat4 view;
        };

        struct SceneInfo {
            SceneRendererCamera sceneCamera;
            EnvironmentLight sceneEnvironmentLight;
            bool showSkybox = true;
            bool enableShadows = true;
            glm::vec4 meshOutlineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        };

        struct Statistics {
            uint32_t drawCalls = 0;
            uint32_t quadCount = 0;
            uint32_t lineCount = 0;
            uint32_t circleCount = 0;

            uint32_t getTotalVertexCount() const {
                return quadCount * 4 + lineCount * 2 + circleCount * 4;
            }

            uint32_t getTotalIndexCount() const {
                return quadCount * 6 + circleCount * 6;
            }
        };

        SceneRenderer();

        void beginScene(const Camera &camera, const glm::mat4 &transform);

        void beginScene(const EditorCamera &camera);

        void beginScene(const SceneRendererCamera &camera);

        void endScene();

        void drawSprite(const glm::mat4 &transform, SpriteRendererComponent &sprite, int objectID = -1);

        void drawString(const std::string &string, const glm::mat4 &transform, const TextComponent &component,
                        int objectID = -1);

        void drawCircle(const glm::mat4 &transform, const glm::vec4 &color, float thickness = 1.0f, float fade = 0.005f,
                        int objectID = -1);

        void drawRect(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color, int objectId = -1);

        void drawRect(const glm::mat4 &transform, const glm::vec4 &color, int objectId = -1);

        void drawQuadBillboard(const glm::vec3 &translation, const glm::vec2 &size, const glm::vec4 &color,
                               int objectId = -1);

        void drawQuadBillboard(const glm::vec3 &position, const glm::vec2 &size,
                               const std::shared_ptr<Texture2D> &texture, float tilingFactor = 1.0f,
                               const glm::vec4 &tintColor = glm::vec4(1.0f), int objectId = -1);

        void drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color);

        void setLineWidth(float thickness);

        void submitMesh(MeshComponent &meshComponent, glm::mat4 transform, int objectId = -1, bool drawOutline = false);

        void setScene(std::shared_ptr<Scene> scene) {
            m_scene = scene;
        }

        std::shared_ptr<Scene> getScene() const {
            return m_scene;
        }

        std::shared_ptr<DebugRenderer> GetDebugRenderer() const {
            return m_debugRenderer;
        }

        SceneInfo &getSceneInfo() {
            return m_sceneData;
        }

        Statistics getStatistics() const;

    private:
        void GeometryPass();

        void OutlinePass();

        void SkyboxPass();

        void FlushDrawList();

    private:
        std::shared_ptr<DebugRenderer> m_debugRenderer;

        std::shared_ptr<Scene> m_scene;

        std::vector<MeshDrawCommand> s_MeshDrawList;

        std::unique_ptr<TextureCube> m_skybox = nullptr;
        std::shared_ptr<VertexArray> m_cubeVA = nullptr;

        std::shared_ptr<Pipeline> m_MeshPipeline;
        std::shared_ptr<Pipeline> m_SkyboxPipeline;

        RenderGraph m_RenderGraph;
        RenderCommandQueue m_CommandQueue;
        SceneInfo m_sceneData;
    };
} // namespace Fermion
