#include "ViewportPanel.hpp"
#include "Scene/EntityManager.hpp"
#include "Math/Math.hpp"
#include "ImGui/BosonUI.hpp"

#include <imgui.h>
#include <ImGuizmo.h>

#define IMVIEWGUIZMO_IMPLEMENTATION
#include <ImViewGuizmo/ImViewGuizmo.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Fermion
{
    void ViewportPanel::onImGuiRender(const Context &ctx)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.117f, 0.117f, 0.117f, 1.0f});
        ImGui::Begin("Viewport");
        const auto viewportOffset = ImGui::GetCursorPos();
        m_viewportTabBarHeight = viewportOffset.y;
        m_viewportFocused = ImGui::IsWindowFocused();
        m_viewportHovered = ImGui::IsWindowHovered();

        const bool fpsMode = ctx.editorCamera->isFPSMode();
        if (fpsMode)
        {
            m_viewportFocused = true;
            m_viewportHovered = true;
        }

        if (m_viewportHovered)
        {
            ImGui::SetWindowFocus();
        }
        ImGuiIO &io = ImGui::GetIO();

        if (fpsMode)
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
            Application::get().getImGuiLayer()->blockEvents(false);
        }
        else
        {
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            Application::get().getImGuiLayer()->blockEvents(!m_viewportFocused || !m_viewportHovered);
        }
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        const uint32_t textureID = ctx.framebuffer->getColorAttachmentRendererID(0);
        ImGui::Image(reinterpret_cast<void *>(static_cast<intptr_t>(textureID)),
                     ImVec2(viewportPanelSize.x, viewportPanelSize.y),
                     ImVec2(0, 1), ImVec2(1, 0));

        // Accept .fmscene drag-drop
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_SCENE"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0] && m_callbacks.onOpenScene)
                {
                    m_callbacks.onOpenScene(std::string(path));
                }
            }
            ImGui::EndDragDropTarget();
        }
        // Accept .fmproj drag-drop
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("FERMION_PROJECT"))
            {
                const char *path = static_cast<const char *>(payload->Data);
                if (path && path[0] && m_callbacks.onOpenProject)
                {
                    m_callbacks.onOpenProject(std::string(path));
                }
            }
            ImGui::EndDragDropTarget();
        }

        const ImVec2 windowPos = ImGui::GetWindowPos();

        m_viewport.min = {
            windowPos.x + viewportOffset.x,
            windowPos.y + viewportOffset.y};

        m_viewport.max = {
            m_viewport.min.x + viewportPanelSize.x,
            m_viewport.min.y + viewportPanelSize.y};
        // ImGuizmo
        Entity selectedEntity = ctx.selectedEntity;

        if (selectedEntity && m_gizmoType != -1)
        {
            ImGuizmo::SetOrthographic(ctx.editorCamera->getProjectionType() == ProjectionType::Orthographic);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(m_viewport.min.x, m_viewport.min.y, m_viewport.size().x, m_viewport.size().y);
            const glm::mat4 &cameraProjection = ctx.editorCamera->getProjection();
            const glm::mat4 &cameraView = ctx.editorCamera->getViewMatrix();
            auto &transformComponent = selectedEntity.getComponent<TransformComponent>();
            glm::mat4 worldTransform = ctx.activeScene->getEntityManager().getWorldSpaceTransformMatrix(selectedEntity);

            bool snap = Input::isKeyPressed(KeyCode::LeftAlt);
            float snapValue = 0.5f;
            if (m_gizmoType == ImGuizmo::OPERATION::ROTATE)
                snapValue = 45.0f;

            float snapValues[3] = {snapValue, snapValue, snapValue};
            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                                 (ImGuizmo::OPERATION)m_gizmoType, ImGuizmo::LOCAL, glm::value_ptr(worldTransform), nullptr,
                                 snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::mat4 localTransform = worldTransform;
                Entity parent = ctx.activeScene->getEntityManager().tryGetEntityByUUID(selectedEntity.getParentUUID());
                if (parent)
                {
                    glm::mat4 parentTransform = ctx.activeScene->getEntityManager().getWorldSpaceTransformMatrix(parent);
                    localTransform = glm::inverse(parentTransform) * worldTransform;
                }

                glm::vec3 translation, rotation, scale;
                Math::decomposeTransform(localTransform, translation, rotation, scale);

                switch ((ImGuizmo::OPERATION)m_gizmoType)
                {
                case ImGuizmo::TRANSLATE:
                    transformComponent.translation = translation;
                    break;
                case ImGuizmo::ROTATE:
                    transformComponent.setRotationEuler(rotation);
                    break;
                case ImGuizmo::SCALE:
                    transformComponent.scale = scale;
                    break;
                default:
                    break;
                }
            }
        }
        onOverlayViewportUI(ctx);

        ImGui::End();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    void ViewportPanel::onOverlayViewportUI(const Context &ctx)
    {
        const float iconSize = 28.0f;
        const float padding = 6.0f;
        const ImU32 backgroundColor = IM_COL32(30, 30, 30, 140);
        const ImVec4 orangeMain = ImVec4(0.92f, 0.45f, 0.11f, 0.8f);

        if (ctx.editorCamera->isFPSMode())
        {
            float centerY = m_viewport.size().y / 2.0f;
            ImGui::SetCursorPos(ImVec2(12.0f, centerY - 150.0f));
            ui::verticalProgressBar(ctx.editorCamera->getFPSSpeed(), 0.0f, 30.0f, ImVec2(12, 300.0f));
        }

        ImGuiChildFlags child_flags = ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));

        ImGui::SetCursorPos(ImVec2(10.0f, m_viewportTabBarHeight + 10.0f));
        if (ImGui::BeginChild("GizmoToolbar", ImVec2(0, 0), child_flags, window_flags))
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(ImGui::GetWindowPos(),
                                     ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight()),
                                     backgroundColor, 6.0f);

            auto drawGizmoBtn = [&](const char *label, int type, const char *tooltip)
            {
                bool isActive = (m_gizmoType == type);
                if (isActive)
                    ImGui::PushStyleColor(ImGuiCol_Button, orangeMain);
                else
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                if (ImGui::Button(label, ImVec2(iconSize, iconSize)))
                    m_gizmoType = type;
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                    ImGui::SetTooltip("%s", tooltip);
                ImGui::PopStyleColor();
            };

            drawGizmoBtn("Q", -1, "Select (Q)");
            ImGui::SameLine();
            drawGizmoBtn("W", (int)ImGuizmo::TRANSLATE, "Translate (W)");
            ImGui::SameLine();
            drawGizmoBtn("E", (int)ImGuizmo::ROTATE, "Rotate (E)");
            ImGui::SameLine();
            drawGizmoBtn("R", (int)ImGuizmo::SCALE, "Scale (R)");
            // Vertical separator
            ImGui::SameLine(0.0f, 8.0f);
            {
                ImVec2 cursorScreen = ImGui::GetCursorScreenPos();
                float lineHeight = iconSize;
                draw_list->AddLine(
                    ImVec2(cursorScreen.x, cursorScreen.y + 2.0f),
                    ImVec2(cursorScreen.x, cursorScreen.y + lineHeight - 2.0f),
                    IM_COL32(255, 255, 255, 60), 1.0f);
                ImGui::Dummy(ImVec2(1.0f, lineHeight));
            }
            ImGui::SameLine(0.0f, 8.0f);

            // Scene control buttons
            const bool toolbarEnabled = (bool)ctx.activeScene;
            ImVec4 tintColor = toolbarEnabled ? ImVec4(1, 1, 1, 1) : ImVec4(1, 1, 1, 0.5f);

            bool isEdit = ctx.sceneState == 0;
            bool isPlay = ctx.sceneState == 1;
            bool isSim = ctx.sceneState == 2;

            auto drawImgBtn = [&](const char *id, Texture2D *iconRef, ImVec4 bg, auto func)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, bg);
                auto textureID = (ImTextureID)(uintptr_t)iconRef->getRendererID();
                if (ImGui::ImageButton(id, textureID, ImVec2(iconSize - 6, iconSize - 6), {0, 0}, {1, 1}, {0, 0, 0, 0}, tintColor))
                {
                    if (toolbarEnabled)
                        func();
                }
                ImGui::PopStyleColor();
            };

            if (isEdit)
            {
                drawImgBtn("##Play", ctx.iconPlay, ImVec4(0, 0, 0, 0), [this]()
                           { if (m_callbacks.onPlay) m_callbacks.onPlay(); });
            }
            else if (isPlay)
            {
                drawImgBtn("##Stop", ctx.iconStop, ImVec4(0.8f, 0.1f, 0.1f, 0.6f), [this]()
                           { if (m_callbacks.onStop) m_callbacks.onStop(); });
            }

            ImGui::SameLine();

            if (isEdit)
            {
                drawImgBtn("##Sim", ctx.iconSimulate, ImVec4(0, 0, 0, 0), [this]()
                           { if (m_callbacks.onSimulate) m_callbacks.onSimulate(); });
            }
            else if (isSim)
            {
                drawImgBtn("##StopSim", ctx.iconStop, ImVec4(0.8f, 0.1f, 0.1f, 0.6f), [this]()
                           { if (m_callbacks.onStop) m_callbacks.onStop(); });
            }

            if (isSim && ctx.activeScene)
            {
                ImGui::SameLine();
                bool isPaused = ctx.activeScene->isPaused();
                drawImgBtn("##Pause", ctx.iconPause, isPaused ? orangeMain : ImVec4(0, 0, 0, 0), [&ctx, isPaused]()
                           { ctx.activeScene->setPaused(!isPaused); });

                if (isPaused)
                {
                    ImGui::SameLine();
                    drawImgBtn("##Step", ctx.iconStep, ImVec4(0, 0, 0, 0), [&ctx]()
                               { ctx.activeScene->step(); });
                }
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(3);
        // imViewGuizmo
        if (!ctx.editorCamera->isFPSMode() && ctx.sceneState != 1) // not Play
        {
            ImViewGuizmo::BeginFrame();

            auto &style = ImViewGuizmo::GetStyle();
            style.scale = 0.65f;

            const float gizmoSize = 256.0f * style.scale;
            const float inset = gizmoSize * 0.1f;

            ImVec2 gizmoCenter = ImVec2(
                m_viewport.max.x - gizmoSize * 0.5f + inset,
                m_viewport.min.y + gizmoSize * 0.5f - inset);

            glm::vec3 cameraPos = ctx.editorCamera->getPosition();
            glm::quat cameraRot = ctx.editorCamera->getOrientation();
            glm::vec3 pivot = ctx.editorCamera->getFocalPoint();

            bool modified = ImViewGuizmo::Rotate(cameraPos, cameraRot, pivot, gizmoCenter);

            if (modified)
            {
                glm::vec3 forward = cameraRot * glm::vec3(0.0f, 0.0f, -1.0f);
                float newPitch = glm::asin(glm::clamp(-forward.y, -1.0f, 1.0f));
                float newYaw = glm::atan(forward.x, -forward.z);

                float distance = glm::length(cameraPos - pivot);
                ctx.editorCamera->setDistance(distance);
                ctx.editorCamera->setFocalPoint(pivot);
                ctx.editorCamera->setYawPitch(newYaw, newPitch);
            }
        }
    }
    void ViewportPanel::updateMousePicking(const Context &ctx)
    {
        m_hoveredEntity = {};

        if (!m_viewport.isValid())
            return;

        const auto [mx, my] = ImGui::GetMousePos();
        const glm::vec2 mouseScreen{mx, my};

        if (!m_viewport.contains(mouseScreen))
            return;

        const glm::vec2 local = mouseScreen - m_viewport.min;
        const glm::vec2 size = m_viewport.size();

        const int pixelX = static_cast<int>(local.x);
        const int pixelY = static_cast<int>(size.y - local.y);

        int entityID = -1;
        if (ctx.framebuffer)
            entityID = ctx.framebuffer->readPixel(1, pixelX, pixelY);

        Entity hovered{static_cast<entt::entity>(entityID), ctx.activeScene.get()};
        if (!hovered && ctx.viewportRenderer)
        {
            const auto &sceneInfo = ctx.viewportRenderer->getSceneInfo();
            if (sceneInfo.renderMode == SceneRenderer::RenderMode::DeferredHybrid)
            {
                if (auto gbuffer = ctx.viewportRenderer->getGBufferFramebuffer())
                {
                    entityID = gbuffer->readPixel(static_cast<uint32_t>(SceneRenderer::GBufferAttachment::ObjectID), pixelX, pixelY);
                    hovered = Entity{static_cast<entt::entity>(entityID), ctx.activeScene.get()};
                }
            }
        }

        if (hovered.isValid())
            m_hoveredEntity = hovered;
    }
} // namespace Fermion
