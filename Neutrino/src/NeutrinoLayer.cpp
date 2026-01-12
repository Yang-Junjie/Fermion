#include "NeutrinoLayer.hpp"

#include "Project/Project.hpp"
#include "Project/ProjectSerializer.hpp"
#include "Scene/SceneSerializer.hpp"

NeutrinoLayer::NeutrinoLayer() : Fermion::Layer("NeutrinoLayer")
{
}

void NeutrinoLayer::onAttach()
{
    m_sceneRenderer = std::make_shared<Fermion::SceneRenderer>();
    openProject();
    if (m_runtimeScene)
    {
        const auto &window = Fermion::Application::get().getWindow();
        m_lastViewportWidth = window.getWidth();
        m_lastViewportHeight = window.getHeight();
        m_runtimeScene->onViewportResize(m_lastViewportWidth, m_lastViewportHeight);
        m_sceneRenderer->setScene(m_runtimeScene);
    }
    onScenePlay();
}

void NeutrinoLayer::onDetach()
{
    if (m_runtimeScene)
        onSceneStop();

    m_sceneRenderer.reset();
    m_runtimeScene.reset();
}

void NeutrinoLayer::onUpdate(Fermion::Timestep dt)
{
    if (!m_runtimeScene || !m_sceneRenderer)
        return;

    const auto &window = Fermion::Application::get().getWindow();
    const uint32_t windowWidth = window.getWidth();
    const uint32_t windowHeight = window.getHeight();
    if (windowWidth > 0 && windowHeight > 0 &&
        (windowWidth != m_lastViewportWidth || windowHeight != m_lastViewportHeight))
    {
        m_lastViewportWidth = windowWidth;
        m_lastViewportHeight = windowHeight;
        m_runtimeScene->onViewportResize(windowWidth, windowHeight);
    }

    Fermion::RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
    Fermion::RenderCommand::clear();

    m_runtimeScene->onUpdateRuntime(m_sceneRenderer, dt, true);
}

void NeutrinoLayer::onEvent(Fermion::IEvent &event)
{
    Fermion::EventDispatcher dispatcher(event);
    dispatcher.dispatch<Fermion::KeyPressedEvent>(
        [this](Fermion::KeyPressedEvent &e)
        { return onKeyPressedEvent(e); });
}

bool NeutrinoLayer::onKeyPressedEvent(const Fermion::KeyPressedEvent &e)
{
    if (e.getKeyCode() == Fermion::KeyCode::Escape)
    {
        Fermion::Application::get().close();
        return true;
    }

    return false;
}

void NeutrinoLayer::onImGuiRender()
{
}

void NeutrinoLayer::openProject()
{
    const auto runtimeRoot = std::filesystem::current_path().parent_path();

    const auto dummyProject = std::make_shared<Fermion::Project>();
    Fermion::ProjectSerializer serializer(dummyProject);
    const auto runtimeFile = runtimeRoot / "Project.fdat";

    std::string projectName = serializer.deserializeRuntime(runtimeFile);
    FERMION_ASSERT(!projectName.empty(), "Failed to read project name from Project.fdat!");

    std::filesystem::path fmprojPath = runtimeRoot / (projectName + ".fmproj");

    m_project = Fermion::Project::loadProject(fmprojPath);
    FERMION_ASSERT(m_project != nullptr, "Failed to load project!");

    const auto &config = m_project->getConfig();
    FERMION_ASSERT(!config.startScene.empty(), "Project start scene is not set!");

    loadScene(config.startScene.string());
}

void NeutrinoLayer::loadScene(const std::string_view &filepath)
{
    m_runtimeScene = std::make_shared<Fermion::Scene>();

    Fermion::SceneSerializer serializer(m_runtimeScene);
    FERMION_ASSERT(serializer.deserialize(filepath), "Failed to deserialize runtime scene!");

    const auto &window = Fermion::Application::get().getWindow();
    const uint32_t windowWidth = window.getWidth();
    const uint32_t windowHeight = window.getHeight();
    if (windowWidth > 0 && windowHeight > 0)
    {
        m_lastViewportWidth = windowWidth;
        m_lastViewportHeight = windowHeight;
        m_runtimeScene->onViewportResize(windowWidth, windowHeight);
    }

    if (m_sceneRenderer)
        m_sceneRenderer->setScene(m_runtimeScene);
}

void NeutrinoLayer::onScenePlay() const
{
    if (m_runtimeScene)
        m_runtimeScene->onRuntimeStart();
}

void NeutrinoLayer::onSceneStop() const
{
    if (m_runtimeScene && m_runtimeScene->isRunning())
        m_runtimeScene->onRuntimeStop();
}
