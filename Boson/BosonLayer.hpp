#pragma once
#include "Fermion.hpp"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class BosonLayer : public Fermion::Layer
{
public:
    BosonLayer(const std::string &name = "BosonLayer") ;
    virtual ~BosonLayer() = default;

    virtual void onAttach() override;
    virtual void onDetach() override;
    virtual void onUpdate(Fermion::Timestep dt) override;

    virtual void onEvent(Fermion::IEvent &event) override;

    virtual void onImGuiRender() override;
private:
    std::shared_ptr<Fermion::VertexArray> m_squareVA;
    std::shared_ptr<Fermion::Shader> m_flatColorShader;
    glm::vec4 m_squareColor = {0.2, 0.3, 0.8, 1.0};

    std::shared_ptr<Fermion::Texture2D> m_checkerboardTexture;
    std::shared_ptr<Fermion::Texture2D> m_spriteSheet;
    std::shared_ptr<Fermion::SubTexture2D> m_textureStairs;
    std::shared_ptr<Fermion::SubTexture2D> m_textureBarrel;
    std::shared_ptr<Fermion::SubTexture2D> m_textureTree;

    Fermion::OrthographicCameraController m_cameraController;

    std::shared_ptr<Fermion::Framebuffer> m_framebuffer;

    glm::vec2 m_viewportSize { 0.0f, 0.0f };
    bool m_viewportFocused = false;
    bool m_viewportHovered = false;
};
