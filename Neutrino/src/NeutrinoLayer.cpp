#include "NeutrinoLayer.hpp"

#include "Project/Project.hpp"
#include "Scene/SceneSerializer.hpp"

NeutrinoLayer::NeutrinoLayer(const std::filesystem::path &projectPath)
    : Fermion::Layer("NeutrinoLayer"), m_projectPath(projectPath)
{
}

void NeutrinoLayer::onAttach()
{
    m_sceneRenderer = std::make_shared<Fermion::SceneRenderer>();
    openProject();
    if (m_runtimeScene)
    {
        auto &window = Fermion::Engine::get().getWindow();
        m_runtimeScene->onViewportResize(window.getWidth(), window.getHeight());
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
    dispatcher.dispatch<Fermion::WindowResizeEvent>(
        [this](Fermion::WindowResizeEvent &e)
        {
            if (m_runtimeScene)
                m_runtimeScene->onViewportResize(e.getWidth(), e.getHeight());
            return false;
        });
}

bool NeutrinoLayer::onKeyPressedEvent(Fermion::KeyPressedEvent &e)
{
    if (e.getKeyCode() == Fermion::KeyCode::Escape)
    {
        Fermion::Engine::get().close();
        return true;
    }

    return false;
}

void NeutrinoLayer::onImGuiRender()
{
}

void NeutrinoLayer::openProject()
{

    m_project = Fermion::Project::loadProject(m_projectPath);
    FERMION_ASSERT(m_project != nullptr, "Failed to load project!");

    const auto &config = m_project->getConfig();
    FERMION_ASSERT(!config.startScene.empty(), "Project start scene is not set!");

    loadScene(config.startScene);
}

void NeutrinoLayer::loadScene(const std::filesystem::path &filepath)
{
    m_runtimeScene = std::make_shared<Fermion::Scene>();

    Fermion::SceneSerializer serializer(m_runtimeScene);
    bool ok = serializer.deserialize(filepath);
    FERMION_ASSERT(ok, "Failed to deserialize runtime scene!");

    if (m_sceneRenderer)
        m_sceneRenderer->setScene(m_runtimeScene);
}

void NeutrinoLayer::onScenePlay()
{
    if (m_runtimeScene)
        m_runtimeScene->onRuntimeStart();
}

void NeutrinoLayer::onSceneStop()
{
    if (m_runtimeScene && m_runtimeScene->isRunning())
        m_runtimeScene->onRuntimeStop();
}
