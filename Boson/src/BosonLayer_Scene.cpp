#include "BosonLayer.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Utils/PlatformUtils.hpp"
#include "Asset/SceneAsset.hpp"

#include <format>

namespace Fermion
{
    void BosonLayer::newScene()
    {
        m_activeScene = std::make_shared<Scene>();
        m_activeScene->onViewportResize(static_cast<uint32_t>(m_viewportPanel.getViewportSize().x),
                                        static_cast<uint32_t>(m_viewportPanel.getViewportSize().y));
        m_editorScene = m_activeScene;
        m_editorScenePath.clear();
        m_editorSceneHandle = AssetHandle{};
        m_sceneHierarchyPanel.setContext(m_activeScene);
        m_viewportRenderer->setScene(m_activeScene);
    }

    void BosonLayer::saveSceneAs()
    {
        std::string defaultDir = "../Boson/assets/scenes/";
        if (auto project = Project::getActive())
        {
            const auto &assetDir = project->getConfig().assetDirectory;
            if (!assetDir.empty())
                defaultDir = assetDir.string();
        }

        auto path = FileDialogs::saveFile(
            "Scene (*.fmscene)\0*.fmscene\0", defaultDir);
        if (path.empty())
            return;

        SceneSerializer serializer(m_editorScene);
        syncEnvironmentSettingsToScene();
        serializer.serialize(path);
        m_editorScenePath = path;

        auto editorAssets = Project::getEditorAssetManager();
        AssetHandle handle = editorAssets->importAsset(path);
        if (static_cast<uint64_t>(handle) != 0)
            m_editorSceneHandle = handle;

        Log::Info(std::format("Scene saved successfully! Path: {}",
                              path.string()));
    }
    void BosonLayer::saveScene()
    {
        if (!m_editorScenePath.empty())
        {
            SceneSerializer serializer(m_editorScene);
            syncEnvironmentSettingsToScene();
            serializer.serialize(m_editorScenePath);

            auto editorAssets = Project::getEditorAssetManager();
            AssetHandle handle = editorAssets->importAsset(m_editorScenePath);
            if (static_cast<uint64_t>(handle) != 0)
                m_editorSceneHandle = handle;

            Log::Info(std::format("Scene saved successfully! Path: {}",
                                  m_editorScenePath.string()));
        }
        else
        {
            saveSceneAs();
        }
    }

    void BosonLayer::openScene()
    {
        std::string defaultDir = "../Boson/assets/scenes/";
        if (auto project = Project::getActive())
        {
            const auto &assetDir = project->getConfig().assetDirectory;
            if (!assetDir.empty())
                defaultDir = assetDir.string();
        }

        std::filesystem::path path = FileDialogs::openFile(
            "Scene (*.fmscene)\0*.fmscene\0", defaultDir);
        if (!path.empty())
        {
            openScene(path);
        }
    }

    void BosonLayer::openScene(const std::filesystem::path &path)
    {
        if (path.empty())
        {
            Log::Warn("openScene called with empty path");
            return;
        }

        if (m_sceneState != SceneState::Edit)
        {
            onSceneStop();
        }
        auto editorAssets = Project::getEditorAssetManager();
        AssetHandle handle = editorAssets->importAsset(path);
        if (static_cast<uint64_t>(handle) == 0)
        {
            Log::Error(std::format("Scene open failed (invalid asset handle)! Path: {}",
                                   path.string()));
            return;
        }

        const auto sceneAsset = editorAssets->getAsset<SceneAsset>(handle);
        if (!sceneAsset || !sceneAsset->scene)
        {
            Log::Error(std::format("Scene open failed (asset load failed)! Path: {}",
                                   path.string()));
            return;
        }

        if (std::shared_ptr<Scene> newScene = sceneAsset->scene)
        {
            m_editorSceneHandle = handle;
            m_editorScene = newScene;
            m_editorScene->onViewportResize(static_cast<uint32_t>(m_viewportPanel.getViewportSize().x),
                                            static_cast<uint32_t>(m_viewportPanel.getViewportSize().y));
            m_activeScene = m_editorScene;
            m_editorScenePath = path;
            m_sceneHierarchyPanel.setContext(m_activeScene);
            m_viewportRenderer->setScene(m_activeScene);
            syncEnvironmentSettingsFromScene();
            Log::Info(std::format("Scene opened successfully! Path: {}",
                                  path.string()));
        }
        else
        {
            Log::Error(std::format("Scene open failed! Path: {}",
                                   path.string()));
        }
    }

    void BosonLayer::onScenePlay()
    {
        m_viewportPanel.setHoveredEntity({});
        if (m_sceneState == SceneState::Simulate)
            onSceneStop();
        m_sceneState = SceneState::Play;

        m_activeScene = Scene::copy(m_editorScene);
        m_activeScene->onRuntimeStart();
        m_sceneHierarchyPanel.setEditingEnabled(false);
        m_viewportRenderer->setScene(m_activeScene);
        m_sceneHierarchyPanel.setContext(m_activeScene);
    }

    void BosonLayer::onSceneSimulate()
    {
        if (m_sceneState == SceneState::Play)
            onSceneStop();
        m_sceneState = SceneState::Simulate;
        m_activeScene = Scene::copy(m_editorScene);
        m_activeScene->onSimulationStart();
        m_sceneHierarchyPanel.setEditingEnabled(false);
        m_viewportRenderer->setScene(m_activeScene);
        m_sceneHierarchyPanel.setContext(m_activeScene);
    }

    void BosonLayer::onSceneStop()
    {
        if (m_sceneState == SceneState::Play)
            m_activeScene->onRuntimeStop();
        else if (m_sceneState == SceneState::Simulate)
            m_activeScene->onSimulationStop();

        m_sceneState = SceneState::Edit;
        m_activeScene = m_editorScene;
        m_viewportRenderer->setScene(m_activeScene);
        m_sceneHierarchyPanel.setEditingEnabled(true);
        m_sceneHierarchyPanel.setContext(m_activeScene);

        m_viewportPanel.setHoveredEntity({});
    }
    void BosonLayer::syncEnvironmentSettingsToScene()
    {
        if (!m_editorScene || !m_viewportRenderer)
            return;
        auto &rendererEnv = m_viewportRenderer->getSceneInfo().environmentSettings;
        auto &sceneEnv = m_editorScene->getEnvironmentSettings();
        sceneEnv.showSkybox = rendererEnv.showSkybox;
        sceneEnv.enableShadows = rendererEnv.enableShadows;
        sceneEnv.ambientIntensity = rendererEnv.ambientIntensity;
        sceneEnv.shadowMapSize = rendererEnv.shadowMapSize;
        sceneEnv.shadowBias = rendererEnv.shadowBias;
        sceneEnv.shadowSoftness = rendererEnv.shadowSoftness;
        sceneEnv.normalMapStrength = rendererEnv.normalMapStrength;
        sceneEnv.toksvigStrength = rendererEnv.toksvigStrength;
        sceneEnv.useIBL = rendererEnv.useIBL;
    }

    void BosonLayer::syncEnvironmentSettingsFromScene()
    {
        if (!m_editorScene || !m_viewportRenderer)
            return;
        auto &sceneEnv = m_editorScene->getEnvironmentSettings();
        auto &rendererEnv = m_viewportRenderer->getSceneInfo().environmentSettings;
        rendererEnv.showSkybox = sceneEnv.showSkybox;
        rendererEnv.enableShadows = sceneEnv.enableShadows;
        rendererEnv.ambientIntensity = sceneEnv.ambientIntensity;
        rendererEnv.shadowMapSize = sceneEnv.shadowMapSize;
        rendererEnv.shadowBias = sceneEnv.shadowBias;
        rendererEnv.shadowSoftness = sceneEnv.shadowSoftness;
        rendererEnv.normalMapStrength = sceneEnv.normalMapStrength;
        rendererEnv.toksvigStrength = sceneEnv.toksvigStrength;
        rendererEnv.useIBL = sceneEnv.useIBL;
    }
} // namespace Fermion
