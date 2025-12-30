#pragma once

#include "Fermion.hpp"
#include "fmpch.hpp"

struct ProjectInfo {
    std::string Name;
    std::filesystem::path Path;
    std::filesystem::path ProjectFile;
};

class LauncherLayer : public Fermion::Layer {
public:
    explicit LauncherLayer();

    ~LauncherLayer() override = default;

    void onAttach() override;

    void onDetach() override;

    void onUpdate(Fermion::Timestep dt) override;

    void onEvent(Fermion::IEvent &event) override;

    bool onKeyPressedEvent(const Fermion::KeyPressedEvent &e);

    void onImGuiRender() override;

private:
    void drawProjectsListPanel();

    void drawSettingPanel();
    void scanProjects();
    void openProject(const ProjectInfo& project);

private:
    std::filesystem::path m_projectsRoot;
    std::vector<ProjectInfo> m_projects;
};
