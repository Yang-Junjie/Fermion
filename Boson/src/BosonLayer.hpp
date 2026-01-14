#pragma once
#include "Fermion.hpp"

#include "Renderer/Camera/EditorCamera.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ContentBrowserPanel.hpp"
#include "Panels/AssetManagerPanel.hpp"
#include "Panels/MenuBarPanel.hpp"
#include "Panels/MaterialEditorPanel.hpp"

#include "Renderer/SceneRenderer.hpp"

#include <filesystem>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Fermion
{
    class BosonLayer : public Layer
    {
        friend class MenuBarPanel;

    public:
        BosonLayer(const std::string &name = "BosonLayer", std::filesystem::path initialProjectPath = {});

        ~BosonLayer() override = default;

        void onAttach() override;

        void onDetach() override;

        void onUpdate(Timestep dt) override;

        void onImGuiRender() override;

        void onEvent(IEvent &event) override;

        void openProject(const std::filesystem::path &path);

    private:
        bool onKeyPressedEvent(KeyPressedEvent &e);

        bool onMouseButtonPressedEvent(const MouseButtonPressedEvent &e);

        void onOverlayRender() const;

        void newProject();

        void openProject();

        void saveProject();

        void newScene();

        void saveSceneAs();

        void saveScene();

        void openScene();

        void openScene(const std::filesystem::path &path);

        void onScenePlay();

        void onSceneSimulate();

        void onSceneStop();

        void onDuplicateEntity();

        // ImGui Panels
        void onUIToolPanel();

        void onHelpPanel();

        void openAboutWindow();
        void openMaterialEditorPanel();

        void onSettingsPanel();

        void onViewportPanel();

        void verticalProgressBar(float value, float minValue, float maxValue, ImVec2 size);

        void updateMousePicking();

    private:
        struct ViewportBounds
        {
            glm::vec2 min; // top-left in screen space
            glm::vec2 max; // bottom-right in screen space

            [[nodiscard]]
            glm::vec2 size() const
            {
                return max - min;
            }

            [[nodiscard]]
            bool isValid() const
            {
                const glm::vec2 s = size();
                return s.x > 0.0f && s.y > 0.0f;
            }

            [[nodiscard]]
            bool contains(const glm::vec2 &p) const
            {
                return p.x >= min.x && p.y >= min.y &&
                       p.x < max.x && p.y < max.y;
            }
        };

        SceneHierarchyPanel m_sceneHierarchyPanel;
        MaterialEditorPanel m_materialEditorPanel;
        ContentBrowserPanel m_contentBrowserPanel;
        AssetManagerPanel m_assetManagerPanel;
        MenuBarPanel m_menuBarPanel;

        bool m_isAboutWindowOpen = false;
        bool m_isMaterialEditorOpen = false;

        std::shared_ptr<Framebuffer> m_framebuffer;


        glm::vec2 m_viewportSize{0.0f, 0.0f};

        ViewportBounds m_viewport = {.min{0.0f, 0.0f}, .max{0.0f, 0.0f}};
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;

        enum class SceneState
        {
            Edit = 0,
            Play = 1,
            Simulate = 2
        };

        SceneState m_sceneState = SceneState::Edit;
        std::shared_ptr<Scene> m_activeScene, m_editorScene, m_runtimeScene;
        std::unique_ptr<Texture2D> m_iconStop, m_iconPlay, m_iconPause, m_iconStep, m_iconSimulate;
        std::filesystem::path m_editorScenePath;
        AssetHandle m_editorSceneHandle{};

        std::shared_ptr<SceneRenderer> m_viewportRenderer;

        EditorCamera m_editorCamera;

        Entity m_hoveredEntity;

        int m_gizmoType = -1;
        bool m_primaryCamera = true;
        bool m_showPhysicsColliders = false;
        bool m_showRenderEntities = true;

        bool m_isInitialized = false;
        std::filesystem::path m_pendingProjectPath;
    };
} // namespace Fermion
