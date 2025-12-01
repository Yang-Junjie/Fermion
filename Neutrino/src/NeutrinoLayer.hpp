#pragma once

#include "Fermion.hpp"
#include "fmpch.hpp"

#include <filesystem>

class NeutrinoLayer : public Fermion::Layer
{
public:
    explicit NeutrinoLayer(const std::filesystem::path &projectPath);
    virtual ~NeutrinoLayer() = default;

    void onAttach() override;
    void onDetach() override;
    void onUpdate(Fermion::Timestep dt) override;

    void onEvent(Fermion::IEvent &event) override;
    bool onKeyPressedEvent(Fermion::KeyPressedEvent &e);

    void onImGuiRender() override;

private:
    void openProject();
    void loadScene(const std::filesystem::path &filepath);

    void onScenePlay();
    void onSceneStop();

private:
    std::shared_ptr<Fermion::Scene> m_runtimeScene;
    std::shared_ptr<Fermion::SceneRenderer> m_sceneRenderer;
    std::shared_ptr<Fermion::Project> m_project;
    std::filesystem::path m_projectPath;
};
