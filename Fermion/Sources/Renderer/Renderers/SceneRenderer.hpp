#pragma once
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "DebugRenderer.hpp"
#include "RenderContext.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderGraphLegacy.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include <array>
#include <vector>
#include "Renderer/RenderCommandQueue.hpp"
#include "ProceduralSkyGenerator.hpp"

namespace Fermion
{
    class DebugRenderer;
    class EnvironmentRenderer;
    class ShadowMapRenderer;
    class GBufferRenderer;
    class SSGIRenderer;
    class GTAORenderer;
    class DeferredLightingRenderer;
    class ForwardRenderer;
    class OutlineRenderer;
    class PostProcessRenderer;
    class InfiniteGridRenderer;
    class UniformBuffer;

    class SceneRenderer
    {
    public:
        enum class GBufferAttachment : uint32_t
        {
            Albedo = 0,
            Normal = 1,
            Material = 2,
            Emissive = 3,
            ObjectID = 4
        };

        enum class RenderMode : uint8_t
        {
            Forward = 0,
            DeferredHybrid = 1
        };

        enum class GBufferDebugMode : uint8_t
        {
            None = 0,
            Albedo = 1,
            Normal = 2,
            Material = 3,
            Roughness = 4,
            Metallic = 5,
            AO = 6,
            Emissive = 7,
            Depth = 8,
            ObjectID = 9,
            SSGI = 10,
            GTAO = 11
        };

        struct EnvironmentSettings
        {
            bool showSkybox = true;
            bool enableShadows = true;
            float ambientIntensity = 0.1f;

            // Shadow mapping settings
            uint32_t shadowMapSize = 2048;
            float shadowBias = 0.01f;
            float shadowSoftness = 1.0f;
            float normalMapStrength = 1.0f;
            float toksvigStrength = 1.0f;

            bool useIBL = true;
        };
        struct SceneInfo
        {
            SceneRendererCamera sceneCamera;
            EnvironmentLight sceneEnvironmentLight;

            EnvironmentSettings environmentSettings;

            bool enableDepthView = false;
            float depthViewPower = 3.0f;
            glm::vec4 meshOutlineColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            float outlineThickness = 2.0f;

            RenderMode renderMode = RenderMode::DeferredHybrid;
            GBufferDebugMode gbufferDebug = GBufferDebugMode::None;

            // Experimental functionality
            bool enableSSGI = false;
            float ssgiIntensity = 1.0f;
            float ssgiRadius = 1.0f;
            float ssgiBias = 0.05f;
            int ssgiSampleCount = 16;
            bool enableGTAO = false;
            float gtaoIntensity = 1.0f;
            float gtaoRadius = 1.0f;
            float gtaoBias = 0.03f;
            float gtaoPower = 1.25f;
            int gtaoSliceCount = 6;
            int gtaoStepCount = 6;

            // IBL settings
            uint32_t irradianceMapSize = 32;
            uint32_t prefilterMapSize = 128;
            uint32_t brdfLUTSize = 512;
            uint32_t prefilterMaxMipLevels = 5;

            ProceduralSkyGenerator::SkySettings skySettings;

            // Infinite Grid settings
            bool showInfiniteGrid = true;
            int gridPlane = 0; // 0 = XZ, 1 = XY, 2 = YZ
            float gridScale = 3.0f;
            float gridFadeDistance = 500.0f;
            glm::vec4 gridColorThin = glm::vec4(0.5f, 0.5f, 0.5f, 0.4f);
            glm::vec4 gridColorThick = glm::vec4(0.5f, 0.5f, 0.5f, 0.6f);
            glm::vec4 gridAxisColorX = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
            glm::vec4 gridAxisColorZ = glm::vec4(0.2f, 0.2f, 0.9f, 1.0f);

            std::string defaultHdrPath;
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
        ~SceneRenderer();

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

        void submitSkinnedMesh(MeshComponent &meshComponent, AnimatorComponent &animator, glm::mat4 transform, int objectId = -1, bool drawOutline = false);

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

        void setOutlineIDs(const std::vector<int> &ids);

        void resetStatistics();

        RenderStatistics getStatistics() const;

        void loadHDREnvironment(const std::string &hdrPath);

        void generateProceduralSky();

        std::shared_ptr<Framebuffer> getGBufferFramebuffer() const;

        uint32_t getGBufferAttachmentRendererID(GBufferAttachment attachment) const;

        void bindGBufferAttachment(GBufferAttachment attachment, uint32_t slot = 0) const;

    private:
        struct FrameResources
        {
            ResourceHandle shadowMap = ResourceHandle{0};
            ResourceHandle gBuffer = ResourceHandle{0};
            ResourceHandle lightingResult = ResourceHandle{0};
            ResourceHandle sceneDepth = ResourceHandle{0};
            ResourceHandle ssgi = ResourceHandle{0};
            ResourceHandle gtao = ResourceHandle{0};
        };

        struct FrameFlags
        {
            bool useDeferred = false;
            bool useSSGI = false;
            bool useGTAO = false;
            bool showGBufferDebug = false;
            bool hasTransparent = false;
        };

        void updateRenderContext();
        void updateViewState(const SceneRendererCamera &camera);
        void FlushDrawList();

        FrameFlags PrepareFrameFlags() const;
        FrameResources PrepareResources(const FrameFlags &flags);
        void PrepareEnvironmentAndShadows(const FrameResources &resources);
        void RenderDeferredPath(const FrameResources &resources, const FrameFlags &flags);
        void RenderForwardPath(const FrameResources &resources, const FrameFlags &flags);
        void AddPostProcessingPasses(const FrameResources &resources, const FrameFlags &flags);

        void SkyboxPass(ResourceHandle lightingResult);
        void ShadowPass(ResourceHandle shadowMap);

    private:
        std::shared_ptr<DebugRenderer> m_debugRenderer;

        std::unique_ptr<GBufferRenderer> m_gBufferRenderer;
        std::unique_ptr<DeferredLightingRenderer> m_lightingRenderer;
        std::unique_ptr<SSGIRenderer> m_ssgiRenderer;
        std::unique_ptr<GTAORenderer> m_gtaoRenderer;
        std::unique_ptr<ForwardRenderer> m_forwardRenderer;
        std::unique_ptr<OutlineRenderer> m_outlineRenderer;
        std::unique_ptr<PostProcessRenderer> m_postProcessRenderer;
        std::unique_ptr<EnvironmentRenderer> m_environmentRenderer;
        std::unique_ptr<ShadowMapRenderer> m_shadowRenderer;
        std::unique_ptr<ProceduralSkyGenerator> m_proceduralSkyGenerator;
        std::unique_ptr<InfiniteGridRenderer> m_infiniteGridRenderer;

        std::shared_ptr<Scene> m_scene;

        std::vector<MeshDrawCommand> m_meshDrawList;

        RenderContext m_renderContext;

        std::shared_ptr<UniformBuffer> m_cameraUniformBuffer;
        std::shared_ptr<UniformBuffer> m_modelUniformBuffer;
        std::shared_ptr<UniformBuffer> m_lightUniformBuffer;
        std::shared_ptr<UniformBuffer> m_boneUniformBuffer;

        std::shared_ptr<Framebuffer> m_targetFramebuffer;

        RenderGraphLegacy m_renderGraph;
        RenderCommandQueue m_commandQueue;
        SceneInfo m_sceneData;

        RenderStatistics::Renderer3DStatistics m_renderer3DStatistics;
        std::array<glm::vec4, 6> m_cameraFrustumPlanes{};
        bool m_hasCameraFrustum = false;
        std::vector<int> m_outlineIDs;
    };
} // namespace Fermion
