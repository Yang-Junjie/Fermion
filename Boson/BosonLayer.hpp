#pragma once
#include "Fermion.hpp"

#include <imgui.h>

#include "Panels/SceneHierarchyPanel.hpp"
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
        void newScene();
        void saveScene();
        void openScene();

    private:
        glm::vec4 m_squareColor = {0.2, 0.3, 0.8, 1.0};
        OrthographicCameraController m_cameraController;

        std::shared_ptr<Framebuffer> m_framebuffer;

        glm::vec2 m_viewportSize{0.0f, 0.0f};

        bool m_viewportFocused = false;
        bool m_viewportHovered = false;

        std::shared_ptr<Scene> m_activeScene;
        Entity m_squareEntity;

        Entity m_cameraEntity;
        Entity m_secondCameraEntity;

        bool m_primaryCamera = true;

        SceneHierarchyPanel m_sceneHierarchyPanel;

        int m_gizmoType = -1;
    };
}
