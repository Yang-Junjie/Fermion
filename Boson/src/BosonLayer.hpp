#pragma once
#include "Fermion.hpp"

#include "Renderer/Camera/EditorCamera.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ContentBrowserPanel.hpp"
#include "Panels/AssetManagerPanel.hpp"
#include "Panels/MenuBarPanel.hpp"
#include "Panels/MaterialEditorPanel.hpp"
#include "Panels/ViewportPanel.hpp"
#include "Panels/SettingsPanel.hpp"
#include "Panels/OverlayRenderPanel.hpp"

#include "Renderer/Renderers/SceneRenderer.hpp"

#include <filesystem>
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

        void syncEnvironmentSettingsToScene();
        void syncEnvironmentSettingsFromScene();

        // ImGui Panels
        void onHelpPanel();
        void openAboutWindow();
        void openMaterialEditorPanel();

    private:
        ViewportPanel m_viewportPanel;
        SettingsPanel m_settingsPanel;
        OverlayRenderPanel m_overlayRenderPanel;

        SceneHierarchyPanel m_sceneHierarchyPanel;
        MaterialEditorPanel m_materialEditorPanel;
        ContentBrowserPanel m_contentBrowserPanel;
        AssetManagerPanel m_assetManagerPanel;
        MenuBarPanel m_menuBarPanel;

        bool m_isAboutWindowOpen = false;
        bool m_isMaterialEditorOpen = false;

        std::shared_ptr<Framebuffer> m_framebuffer;

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

        bool m_primaryCamera = true;
        bool m_showPhysicsColliders = false;
        bool m_showRenderEntities = true;

        bool m_isInitialized = false;
        std::filesystem::path m_pendingProjectPath;
    };
} // namespace Fermion
