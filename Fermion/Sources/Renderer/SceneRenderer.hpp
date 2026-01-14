#pragma once
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include "Model/Mesh.hpp"
#include "Model/Material.hpp"
#include "Camera/Camera.hpp"
#include "Camera/EditorCamera.hpp"
#include "DebugRenderer.hpp"
#include "Framebuffer.hpp"
#include "RenderPass.hpp"
#include "RenderGraph.hpp"
#include "RenderDrawCommand.hpp"
#include <vector>
#include "RenderCommandQueue.hpp"

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
            float farClip = 0.0f;
            float nearClip = 0.1f;
        };

        struct SceneInfo
        {
            SceneRendererCamera sceneCamera;
            EnvironmentLight sceneEnvironmentLight;
            bool showSkybox = true;
            bool enableShadows = true;
            bool enableDepthView = false;
            float depthViewPower = 3.0f;  
            glm::vec4 meshOutlineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);


            float ambientIntensity = 0.1f;

            // Shadow mapping settings
            uint32_t shadowMapSize = 2048;
            float shadowBias = 0.01f;
            float shadowSoftness = 1.0f;

            // IBL settings
            bool useIBL = true;
            uint32_t irradianceMapSize = 32;
            uint32_t prefilterMapSize = 128;
            uint32_t brdfLUTSize = 512;
            uint32_t prefilterMaxMipLevels = 5;

            std::string hdrPath = "../Boson/projects/Assets/hdr/cedar_bridge_2_2k.hdr";
        };

        struct RenderStatistics
        {
            struct Renderer2DStatistics
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

            struct Renderer3DStatistics
            {
                uint32_t meshCount = 0;
                uint32_t geometryDrawCalls = 0;
                uint32_t shadowDrawCalls = 0;
                uint32_t skyboxDrawCalls = 0;
                uint32_t iblDrawCalls = 0;

                uint32_t getTotalDrawCalls() const
                {
                    return geometryDrawCalls + shadowDrawCalls + skyboxDrawCalls + iblDrawCalls;
                }
            };

            Renderer2DStatistics renderer2D;
            Renderer3DStatistics renderer3D;

            uint32_t getTotalDrawCalls() const
            {
                return renderer2D.drawCalls + renderer3D.getTotalDrawCalls();
            }

            uint32_t getTotalVertexCount() const
            {
                return renderer2D.getTotalVertexCount();
            }

            uint32_t getTotalIndexCount() const
            {
                return renderer2D.getTotalIndexCount();
            }
        };

        SceneRenderer();

        void beginScene(const Camera &camera, const glm::mat4 &transform);

        void beginScene(const EditorCamera &camera);

        void beginScene(const SceneRendererCamera &camera);

        void beginOverlay(const Camera &camera, const glm::mat4 &transform);

        void beginOverlay(const EditorCamera &camera);

        void beginOverlay(const SceneRendererCamera &camera);

        void setTargetFramebuffer(std::shared_ptr<Framebuffer> framebuffer)
        {
            m_targetFramebuffer = framebuffer;
        }

        void endScene();

        void endOverlay();

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

        void drawInfiniteLine(const glm::vec3 &point, const glm::vec3 &direction, const glm::vec4 &color);
        void drawLine(const glm::vec3 &start, const glm::vec3 &end, const glm::vec4 &color);

        void setLineWidth(float thickness);

        void submitMesh(MeshComponent &meshComponent, glm::mat4 transform, int objectId = -1, bool drawOutline = false);
        void setScene(std::shared_ptr<Scene> scene)
        {
            m_scene = scene;
        }

        std::shared_ptr<Scene> getScene() const
        {
            return m_scene;
        }

        std::shared_ptr<DebugRenderer> GetDebugRenderer() const
        {
            return m_debugRenderer;
        }

        SceneInfo &getSceneInfo()
        {
            return m_sceneData;
        }

        uint32_t getDepthViewRendererID() const;

        void resetStatistics();

        RenderStatistics getStatistics() const;

        void loadHDREnvironment(const std::string& hdrPath);

    private:
        void GeometryPass();

        void OutlinePass();

        void SkyboxPass();

        void ShadowPass();

        void DepthViewPass();

        void FlushDrawList();

        glm::mat4 calculateLightSpaceMatrix(const DirectionalLight &light, float orthoSize = 20.0f);

        void convertEquirectangularToCubemap();
        void initializeIBL();
        void generateIrradianceMap();
        void generatePrefilterMap();
        void generateBRDFLUT();

    private:
        std::shared_ptr<DebugRenderer> m_debugRenderer;

        std::shared_ptr<Scene> m_scene;

        std::vector<MeshDrawCommand> s_MeshDrawList;

        // HDR环境贴图相关
        std::unique_ptr<Texture2D> m_hdrEnvironment = nullptr;
        std::unique_ptr<TextureCube> m_environmentCubemap = nullptr;
        std::shared_ptr<VertexArray> m_cubeVA = nullptr;
        
        std::shared_ptr<VertexArray> m_quadVA = nullptr;  

        std::shared_ptr<VertexArray> m_depthViewQuadVA = nullptr;

        std::shared_ptr<Pipeline> m_MeshPipeline;
        std::shared_ptr<Pipeline> m_PBRMeshPipeline;
        std::shared_ptr<Pipeline> m_SkyboxPipeline;
        std::shared_ptr<Pipeline> m_ShadowPipeline;
        std::shared_ptr<Pipeline> m_DepthViewPipeline;

        // IBL resources
        std::shared_ptr<Pipeline> m_IBLIrradiancePipeline;
        std::shared_ptr<Pipeline> m_IBLPrefilterPipeline;
        std::shared_ptr<Pipeline> m_IBLBRDFPipeline;
        std::shared_ptr<Pipeline> m_EquirectToCubePipeline;

        std::unique_ptr<TextureCube> m_irradianceMap = nullptr;
        std::unique_ptr<TextureCube> m_prefilterMap = nullptr;
        std::unique_ptr<Texture2D> m_brdfLUT = nullptr;

        bool m_iblInitialized = false;
        bool m_environmentLoaded = false;

        std::shared_ptr<Framebuffer> m_shadowMapFB;
        std::shared_ptr<Framebuffer> m_targetFramebuffer;
        glm::mat4 m_lightSpaceMatrix;

        RenderGraph m_RenderGraph;
        RenderCommandQueue m_CommandQueue;
        SceneInfo m_sceneData;

        RenderStatistics::Renderer3DStatistics m_renderer3DStatistics;
    };
} // namespace Fermion
