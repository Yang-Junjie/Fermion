#pragma once
#include "Fermion.hpp"

#include "Renderer/EditorCamera.hpp"
#include "Panels/SceneHierarchyPanel.hpp"
#include "Panels/ContentBrowserPanel.hpp"

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

        virtual void onEvent(IEvent &event) override;
        virtual void onImGuiRender() override;

    private:
        bool onKeyPressedEvent(KeyPressedEvent &e);
        bool onMouseButtonPressedEvent(MouseButtonPressedEvent &e);

        void onOverlayRender();

        void newScene();
        void saveSceneAs();
        void saveScene();

        void openScene();
        void openScene(const std::filesystem::path &path);

        void onScenePlay();
        void onSceneStop();

        void onDuplicateEntity();

        void UIToolbar();

      

    private:
        OrthographicCameraController m_cameraController;
        std::filesystem::path m_editorScenePath;

        std::shared_ptr<Texture2D> m_iconStop;
        std::shared_ptr<Texture2D> m_iconPlay;
        std::shared_ptr<Framebuffer> m_framebuffer;

        glm::vec2 m_viewportSize{0.0f, 0.0f};
        glm::vec2 m_viewportBounds[2];
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;

        std::shared_ptr<Scene> m_activeScene;
        std::shared_ptr<Scene> m_editorScene, m_runtimeScene;

        Entity m_hoveredEntity;

        bool m_primaryCamera = true;

        EditorCamera m_editorCamera;

        SceneHierarchyPanel m_sceneHierarchyPanel;
        ContentBrowserPanel m_contentBrowserPanel;

        int m_gizmoType = -1;

        enum class SceneState
        {
            Edit = 0,
            Play = 1
        };
        SceneState m_sceneState = SceneState::Edit;
        bool m_showPhysicsColliders = false;
    };
}
