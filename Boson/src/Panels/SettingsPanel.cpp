#include "SettingsPanel.hpp"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace Fermion
{
    static std::shared_ptr<Font> s_SettingsFont;

    void SettingsPanel::onImGuiRender(const Context &ctx)
    {
        if (!s_SettingsFont)
            s_SettingsFont = Font::getDefault();

        renderRendererInfo(ctx);
        renderEnvironmentSettings(ctx);
        renderDebugSettings(ctx);
    }

    void SettingsPanel::renderRendererInfo(const Context &ctx)
    {
        ImGui::Begin("Renderer Info");
        ImGui::SeparatorText("Device Info");
        DeviceInfo info = Application::get().getWindow().getDeviceInfo();
        ImGui::Text("vendor: %s", info.vendor.c_str());
        ImGui::Text("renderer: %s", info.renderer.c_str());
        ImGui::Text("version: %s", info.version.c_str());
        float frameTimeMs = Application::get().getTimestep().getMilliseconds();
        static float avgFrameTimeMs = frameTimeMs;
        const float smoothing = 0.01f;

        avgFrameTimeMs = avgFrameTimeMs * (1.0f - smoothing) + frameTimeMs * smoothing;

        ImGui::Text("Frame time: %.3f ms", avgFrameTimeMs);
        ImGui::Text("FPS: %.1f FPS", 1000.0f / avgFrameTimeMs);

        ImGui::SeparatorText("Renderer Statistics");
        const SceneRenderer::RenderStatistics stats = ctx.viewportRenderer
                                                          ? ctx.viewportRenderer->getStatistics()
                                                          : SceneRenderer::RenderStatistics{};

        ImGui::Text("Draw Calls (Total): %u", stats.getTotalDrawCalls());
        ImGui::SeparatorText("Renderer2D");
        ImGui::Text("Draw Calls: %u", stats.renderer2D.drawCalls);
        ImGui::Text("Quads: %u", stats.renderer2D.quadCount);
        ImGui::Text("Lines: %u", stats.renderer2D.lineCount);
        ImGui::Text("Circles: %u", stats.renderer2D.circleCount);
        ImGui::Text("Vertices: %u", stats.getTotalVertexCount());
        ImGui::Text("Indices: %u", stats.getTotalIndexCount());

        ImGui::SeparatorText("Renderer3D");
        ImGui::Text("Meshes: %u", stats.renderer3D.meshCount);
        ImGui::Text("Geometry Draw Calls: %u", stats.renderer3D.geometryDrawCalls);
        ImGui::Text("Shadow Draw Calls: %u", stats.renderer3D.shadowDrawCalls);
        ImGui::Text("Skybox Draw Calls: %u", stats.renderer3D.skyboxDrawCalls);
        ImGui::Text("IBL Draw Calls: %u", stats.renderer3D.iblDrawCalls);
        ImGui::Text("Draw Calls (3D Total): %u", stats.renderer3D.getTotalDrawCalls());
        ImGui::SeparatorText("Editor Camera Info");
        ImGui::Text("FPS speed: %.2f", ctx.editorCamera->getFPSSpeed());

        const char *projectionTypes[] = {"Perspective", "Orthographic"};
        int projTypeIndex = static_cast<int>(ctx.editorCamera->getProjectionType());
        if (ImGui::Combo("Projection", &projTypeIndex, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
        {
            ctx.editorCamera->setProjectionType(static_cast<ProjectionType>(projTypeIndex));
        }

        if (ctx.editorCamera->getProjectionType() == ProjectionType::Orthographic)
        {
            float orthoSize = ctx.editorCamera->getOrthographicSize();
            if (ImGui::DragFloat("Ortho Size", &orthoSize, 0.1f, 0.1f, 200.0f))
            {
                ctx.editorCamera->setOrthographicSize(orthoSize);
            }
        }

        ImGui::End();
    }

    void SettingsPanel::renderEnvironmentSettings(const Context &ctx)
    {
        ImGui::Begin("Environment Settings");
        auto &sceneInfo = ctx.viewportRenderer->getSceneInfo();
        ImGui::Button("Drag HDR to load");
        const char *hdrPath = nullptr;
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_HDR"))
            {
                hdrPath = static_cast<const char *>(payload->Data);
                if (hdrPath && hdrPath[0])
                {
                    ctx.viewportRenderer->loadHDREnvironment(hdrPath);
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::TextWrapped("HDR: %s",
                           hdrPath
                               ? hdrPath
                               : std::filesystem::path(sceneInfo.defaultHdrPath).filename().string().c_str());
        ImGui::Checkbox("showSkybox", &sceneInfo.environmentSettings.showSkybox);
        ImGui::Checkbox("enable shadows", &sceneInfo.environmentSettings.enableShadows);
        ImGui::Checkbox("use IBL", &sceneInfo.environmentSettings.useIBL);
        ImGui::DragFloat("Ambient Intensity", &sceneInfo.environmentSettings.ambientIntensity, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Normal Map Strength", &sceneInfo.environmentSettings.normalMapStrength, 0.1f, 0.0f, 5.0f);
        ImGui::DragFloat("Toksvig Strength", &sceneInfo.environmentSettings.toksvigStrength, 0.05f, 0.0f, 4.0f);

        ImGui::SeparatorText("SSGI");
        if (ImGui::Checkbox("Enable SSGI", &sceneInfo.enableSSGI))
        {
            if (sceneInfo.enableSSGI)
                sceneInfo.renderMode = SceneRenderer::RenderMode::DeferredHybrid;
        }
        if (sceneInfo.enableSSGI)
        {
            ImGui::Indent();
            ImGui::DragFloat("SSGI Intensity", &sceneInfo.ssgiIntensity, 0.05f, 0.0f, 10.0f);
            ImGui::DragFloat("SSGI Radius", &sceneInfo.ssgiRadius, 0.05f, 0.1f, 10.0f);
            ImGui::DragFloat("SSGI Bias", &sceneInfo.ssgiBias, 0.001f, 0.0f, 1.0f);
            ImGui::SliderInt("SSGI Samples", &sceneInfo.ssgiSampleCount, 1, 1024);
            ImGui::Unindent();
        }

        ImGui::SeparatorText("GTAO");
        if (ImGui::Checkbox("Enable GTAO", &sceneInfo.enableGTAO))
        {
            if (sceneInfo.enableGTAO)
                sceneInfo.renderMode = SceneRenderer::RenderMode::DeferredHybrid;
        }
        if (sceneInfo.enableGTAO)
        {
            ImGui::Indent();
            ImGui::DragFloat("GTAO Intensity", &sceneInfo.gtaoIntensity, 0.05f, 0.0f, 4.0f);
            ImGui::DragFloat("GTAO Radius", &sceneInfo.gtaoRadius, 0.05f, 0.1f, 10.0f);
            ImGui::DragFloat("GTAO Bias", &sceneInfo.gtaoBias, 0.001f, 0.0f, 0.5f);
            ImGui::DragFloat("GTAO Power", &sceneInfo.gtaoPower, 0.05f, 0.1f, 4.0f);
            ImGui::SliderInt("GTAO Slices", &sceneInfo.gtaoSliceCount, 1, 12);
            ImGui::SliderInt("GTAO Steps", &sceneInfo.gtaoStepCount, 1, 16);
            ImGui::Unindent();
        }

        ImGui::Separator();
        ImGui::DragFloat("Shadow Bias", &sceneInfo.environmentSettings.shadowBias, 0.0001f, 0.0f, 0.1f);
        ImGui::Separator();
        static const float kShadowSoftnessLevels[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f};
        static int softnessIndex = 0;

        ImGui::Text("Shadow Softness = %.1f", sceneInfo.environmentSettings.shadowSoftness);
        if (ImGui::SliderInt(
                "##Shadow Softness",
                &softnessIndex,
                0,
                IM_ARRAYSIZE(kShadowSoftnessLevels) - 1))
        {
            sceneInfo.environmentSettings.shadowSoftness = kShadowSoftnessLevels[softnessIndex];
        }
        ImGui::Separator();
        static const float kShadowMapSizeLevels[] = {1024, 2048, 3072, 4096, 5120};
        static int shadowMapSizeIndex = 0;
        ImGui::Text("Shadow MapSize = %.u", sceneInfo.environmentSettings.shadowMapSize);
        if (ImGui::SliderInt(
                "##ShadowMapSize",
                &shadowMapSizeIndex,
                0,
                IM_ARRAYSIZE(kShadowMapSizeLevels) - 1))
        {
            sceneInfo.environmentSettings.shadowMapSize = kShadowMapSizeLevels[shadowMapSizeIndex];
        }
        ImGui::End();
    }

    void SettingsPanel::renderDebugSettings(const Context &ctx)
    {
        std::string name = "None";
        if (ctx.hoveredEntity && ctx.hoveredEntity.hasComponent<TagComponent>())
        {
            name = ctx.hoveredEntity.getComponent<TagComponent>().tag;
        }
        ImGui::Begin("Debug Settings");

        ImGui::Text("hovered entity: %s", name.c_str());
        ImGui::Text("m_viewportFocused: %s", ctx.viewportFocused ? "yes" : "no");

        ImGui::Separator();
        auto &sceneInfo = ctx.viewportRenderer->getSceneInfo();
        const char *renderModes[] = {"Forward", "Deferred Hybrid"};
        int renderModeIndex = static_cast<int>(sceneInfo.renderMode);
        if (ImGui::Combo("Render Mode", &renderModeIndex, renderModes, IM_ARRAYSIZE(renderModes)))
        {
            sceneInfo.renderMode = static_cast<SceneRenderer::RenderMode>(renderModeIndex);
            if (sceneInfo.renderMode == SceneRenderer::RenderMode::Forward)
                sceneInfo.gbufferDebug = SceneRenderer::GBufferDebugMode::None;
        }

        const char *gbufferModes[] = {"None", "Albedo", "Normal", "Material", "Roughness", "Metallic", "AO", "Emissive", "Depth", "ObjectID", "SSGI", "GTAO"};
        int gbufferModeIndex = static_cast<int>(sceneInfo.gbufferDebug);
        if (ImGui::Combo("GBuffer Debug", &gbufferModeIndex, gbufferModes, IM_ARRAYSIZE(gbufferModes)))
        {
            sceneInfo.gbufferDebug = static_cast<SceneRenderer::GBufferDebugMode>(gbufferModeIndex);
            if (sceneInfo.gbufferDebug != SceneRenderer::GBufferDebugMode::None)
                sceneInfo.renderMode = SceneRenderer::RenderMode::DeferredHybrid;
        }
        ImGui::Checkbox("show depth buffer", &ctx.viewportRenderer->getSceneInfo().enableDepthView);
        if (ctx.viewportRenderer->getSceneInfo().enableDepthView)
        {
            ImGui::Indent();
            ImGui::SliderFloat("Depth Power", &ctx.viewportRenderer->getSceneInfo().depthViewPower, 0.1f, 10.0f, "%.2f");
            ImGui::Unindent();
        }
        ImGui::Checkbox("showPhysicsDebug", ctx.showPhysicsDebug);
        ImGui::Checkbox("showRenderEntities", ctx.showRenderEntities);

        ImGui::SeparatorText("Infinite Grid Settings");
        ImGui::Checkbox("Show Infinite Grid", &sceneInfo.showInfiniteGrid);
        if (sceneInfo.showInfiniteGrid)
        {
            ImGui::Indent();
            const char *planeNames[] = {"XZ (Horizontal)", "XY (Front)", "YZ (Side)"};
            ImGui::Combo("Grid Plane", &sceneInfo.gridPlane, planeNames, 3);
            ImGui::Unindent();
        }

        ImGui::SeparatorText("Outline Settings");
        ImGui::ColorEdit4("Outline Color", glm::value_ptr(sceneInfo.meshOutlineColor));
        ImGui::Separator();
        ImGui::Image((ImTextureID)s_SettingsFont->getAtlasTexture()->getRendererID(),
                     {512, 512}, {0, 1}, {1, 0});
        ImGui::End();
    }
} // namespace Fermion
