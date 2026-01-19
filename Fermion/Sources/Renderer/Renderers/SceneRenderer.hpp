#pragma once
#include "Scene/Components.hpp"
#include "Scene/Scene.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Camera/EditorCamera.hpp"
#include "DebugRenderer.hpp"
#include "Renderer/Framebuffer.hpp"
#include "Renderer/RenderGraph.hpp"
#include "Renderer/RenderDrawCommand.hpp"
#include <array>
#include <vector>
#include "Renderer/RenderCommandQueue.hpp"

namespace Fermion
{
    class DebugRenderer;
    class EnvironmentRenderer;
    class ShadowMapRenderer;

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
            ObjectID = 9
        };

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
            float outlineDepthThreshold = 1.0f;
            float outlineNormalThreshold = 2.0f;
            float outlineThickness = 3.0f;

            float ambientIntensity = 0.1f;
            RenderMode renderMode = RenderMode::DeferredHybrid;
            GBufferDebugMode gbufferDebug = GBufferDebugMode::None;

            // Shadow mapping settings
            uint32_t shadowMapSize = 2048;
            float shadowBias = 0.01f;
            float shadowSoftness = 1.0f;
            float normalMapStrength = 1.0f;
            float toksvigStrength = 1.0f;

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

        void loadHDREnvironment(const std::string& hdrPath);

        std::shared_ptr<Framebuffer> getGBufferFramebuffer() const
        {
            return m_gBufferFramebuffer;
        }

        uint32_t getGBufferAttachmentRendererID(GBufferAttachment attachment) const
        {
            return m_gBufferFramebuffer ? m_gBufferFramebuffer->getColorAttachmentRendererID(static_cast<uint32_t>(attachment)) : 0;
        }

        void bindGBufferAttachment(GBufferAttachment attachment, uint32_t slot = 0) const
        {
            if (m_gBufferFramebuffer)
                m_gBufferFramebuffer->bindColorAttachment(static_cast<uint32_t>(attachment), slot);
        }

    private:
        void ForwardPass(ResourceHandle shadowMap, ResourceHandle sceneDepth, ResourceHandle lightingResult);
        void GBufferPass(ResourceHandle gBuffer, ResourceHandle sceneDepth);
        void recordGBufferPass(CommandBuffer &commandBuffer);
        void LightingPass(ResourceHandle gBuffer, ResourceHandle shadowMap, ResourceHandle sceneDepth, ResourceHandle lightingResult);
        void recordLightingPass(CommandBuffer &commandBuffer);
        void GBufferDebugPass(ResourceHandle gBuffer, ResourceHandle sceneDepth);
        void recordGBufferDebugPass(CommandBuffer &commandBuffer);
        void TransparentPass(ResourceHandle shadowMap, ResourceHandle sceneDepth, ResourceHandle lightingResult);
        void recordForwardPass(CommandBuffer &commandBuffer, bool drawTransparent);

        void OutlinePass(ResourceHandle gBuffer, ResourceHandle sceneDepth, ResourceHandle lightingResult);
        void recordOutlinePostProcess(CommandBuffer &commandBuffer, const std::vector<int> &outlineIDs);

        void SkyboxPass(ResourceHandle lightingResult);

        void ShadowPass(ResourceHandle shadowMap);

        void DepthViewPass(ResourceHandle sceneDepth, ResourceHandle lightingResult);

        void FlushDrawList();
        void ensureGBuffer(uint32_t width, uint32_t height);

    private:
        std::shared_ptr<DebugRenderer> m_debugRenderer;
        std::unique_ptr<EnvironmentRenderer> m_environmentRenderer;
        std::unique_ptr<ShadowMapRenderer> m_shadowRenderer;

        std::shared_ptr<Scene> m_scene;

        std::vector<MeshDrawCommand> s_MeshDrawList;

        std::shared_ptr<VertexArray> m_depthViewQuadVA = nullptr;

        std::shared_ptr<Pipeline> m_MeshPipeline;
        std::shared_ptr<Pipeline> m_PBRMeshPipeline;
        std::shared_ptr<Pipeline> m_GBufferMeshPipeline;
        std::shared_ptr<Pipeline> m_GBufferPBRMeshPipeline;
        std::shared_ptr<Pipeline> m_DeferredLightingPipeline;
        std::shared_ptr<Pipeline> m_GBufferDebugPipeline;
        std::shared_ptr<Pipeline> m_GBufferOutlinePipeline;
        std::shared_ptr<Pipeline> m_DepthViewPipeline;
        std::shared_ptr<Framebuffer> m_targetFramebuffer;
        std::shared_ptr<Framebuffer> m_gBufferFramebuffer;

        RenderGraph m_RenderGraph;
        RenderCommandQueue m_CommandQueue;
        SceneInfo m_sceneData;

        RenderStatistics::Renderer3DStatistics m_renderer3DStatistics;
        std::array<glm::vec4, 6> m_cameraFrustumPlanes{};
        bool m_hasCameraFrustum = false;
        std::vector<int> m_outlineIDs;
    };
} // namespace Fermion
