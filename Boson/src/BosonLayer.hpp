#pragma once
#include "Fermion.hpp"

#include "Renderer/EditorCamera.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ContentBrowserPanel.hpp"
#include "Renderer/SceneRenderer.hpp"

#include <filesystem>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace Fermion
{
    class BosonLayer : public Layer
    {
    public:
        BosonLayer(const std::string &name = "BosonLayer");
        virtual ~BosonLayer() = default;

        virtual void onAttach() override;
        virtual void onDetach() override;
        virtual void onUpdate(Timestep dt) override;

        virtual void onImGuiRender() override;
        virtual void onEvent(IEvent &event) override;

    private:
        bool onKeyPressedEvent(KeyPressedEvent &e);
        bool onMouseButtonPressedEvent(MouseButtonPressedEvent &e);

        void onOverlayRender();

        void newProject();
        void openProject();
        void openProject(const std::filesystem::path &path);
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

        void UIToolbar();

    private:
        SceneHierarchyPanel m_sceneHierarchyPanel;
        ContentBrowserPanel m_contentBrowserPanel;

        std::shared_ptr<Framebuffer> m_framebuffer;
        glm::vec2 m_viewportSize{0.0f, 0.0f};
        glm::vec2 m_viewportBounds[2];
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
        std::shared_ptr<Texture2D> m_iconStop, m_iconPlay, m_iconPause, m_iconStep, m_iconSimulate;
        std::filesystem::path m_editorScenePath;

        std::shared_ptr<SceneRenderer> m_viewportRenderer;

        EditorCamera m_editorCamera;

        Entity m_hoveredEntity;

        int m_gizmoType = -1;
        bool m_primaryCamera = true;
        bool m_showPhysicsColliders = false;
    };
}
