#pragma once
#include "Fermion.hpp"

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
        void setViewportSize(const glm::vec2 &newSize);
        void applyPendingViewportResize();

        std::shared_ptr<VertexArray> m_squareVA;
        std::shared_ptr<Shader> m_flatColorShader;
        glm::vec4 m_squareColor = {0.2, 0.3, 0.8, 1.0};

        std::shared_ptr<Texture2D> m_checkerboardTexture;
        std::shared_ptr<Texture2D> m_spriteSheet;
        std::shared_ptr<SubTexture2D> m_textureStairs;
        std::shared_ptr<SubTexture2D> m_textureBarrel;
        std::shared_ptr<SubTexture2D> m_textureTree;

        OrthographicCameraController m_cameraController;

        std::shared_ptr<Framebuffer> m_framebuffer;

        glm::vec2 m_viewportSize{0.0f, 0.0f};
        glm::vec2 m_pendingViewportSize{0.0f, 0.0f};
        bool m_hasPendingViewportResize = false;
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;

        std::shared_ptr<Scene> m_activeScene;
        entt::entity m_squareEntity;
    };
}
