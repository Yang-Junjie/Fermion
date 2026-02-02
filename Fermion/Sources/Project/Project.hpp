#pragma once

#include <string>
#include <memory>
#include <filesystem>

namespace Fermion
{
    class AssetManagerBase;
    class EditorAssetManager;
    class RuntimeAssetManager;

    struct ProjectConfig
    {
        std::string name = "Untitled";
        std::string version = "1.0.0";
        std::string author = "Your";

        std::filesystem::path startScene;
        std::filesystem::path assetDirectory;
        std::filesystem::path scriptDirectory;
    };

    class Project
    {
    public:
        static const std::filesystem::path &getProjectDirectory()
        {
            return s_activeProject->m_projectDirectory;
        }

        static const std::filesystem::path &getProjectPath()
        {
            return s_activeProject->m_projectPath;
        }

        ProjectConfig &getConfig()
        {
            return m_config;
        }

        static std::shared_ptr<Project> getActive()
        {
            // FERMION_ASSERT(s_activeProject!=nullptr, "Project is not active!");
            return s_activeProject;
        }

        static bool saveActive(const std::filesystem::path &path);

        static std::shared_ptr<Project> newProject();

        static std::shared_ptr<Project> loadProject(const std::filesystem::path &path);

        static void initEditorAssets();

        static void initRuntimeAssets();

        inline static std::shared_ptr<AssetManagerBase> getAssetManager()
        {
            return s_assetManager;
        }

        static std::shared_ptr<EditorAssetManager> getEditorAssetManager();

        static std::shared_ptr<RuntimeAssetManager> getRuntimeAssetManager();

    private:
        ProjectConfig m_config;
        std::filesystem::path m_projectDirectory;
        std::filesystem::path m_projectPath;
        inline static std::shared_ptr<AssetManagerBase> s_assetManager;

        inline static std::shared_ptr<Project> s_activeProject = nullptr;
    };
} // namespace Fermion
