#include "BosonLayer.hpp"
#include "Utils/PlatformUtils.hpp"

#include <algorithm>
#include <cctype>
#include <format>

namespace
{
    bool isProjectDescriptor(const std::filesystem::path &path)
    {
        auto ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c)
                       { return static_cast<char>(std::tolower(c)); });
        return ext == ".fmproj";
    }

    std::filesystem::path findProjectFileInDirectory(const std::filesystem::path &directory)
    {
        std::error_code ec;
        for (std::filesystem::directory_iterator it(directory, ec), end; it != end; it.increment(ec))
        {
            if (ec)
                break;

            std::error_code entryEc;
            if (!it->is_regular_file(entryEc) || entryEc)
                continue;

            if (isProjectDescriptor(it->path()))
                return it->path();
        }
        return {};
    }
} // namespace

namespace Fermion
{
    void BosonLayer::newProject()
    {
        std::filesystem::path path = FileDialogs::saveFile(
            "Project (*.fmproj)\0*.fmproj\0", "../Boson/assets/project/");
        if (path.empty())
        {
            Log::Warn("Project Selection directory is empty!");
            return;
        }
        if (std::filesystem::create_directories(path.parent_path() / "Assets"))
            Log::Info(std::format("Assets directory created successfully! Path: {}",
                                  (path.parent_path() / "Assets").string()));
        else
            Log::Warn(std::format("Assets directory already exists! Path: {}",
                                  (path.parent_path() / "Assets").string()));

        Project::newProject();

        auto &config = Project::getActive()->getConfig();
        config.name = path.stem().string();
        config.startScene = "";
        config.assetDirectory = path.parent_path() / "Assets";
        config.scriptDirectory = config.assetDirectory / "scripts";

        if (Project::saveActive(path))
            Log::Info(std::format("Project created successfully! Path: {}",
                                  Project::getActive()->getProjectPath().string()));
        else
            Log::Error("Project create failed!");

        m_contentBrowserPanel.setBaseDirectory(Project::getActive()->getConfig().assetDirectory);
    }

    void BosonLayer::openProject()
    {
        std::filesystem::path path = FileDialogs::openFile(
            "Project (*.fmproj)\0*.fmproj\0", "../Boson/assets/project/");
        if (path.empty())
        {
            Log::Warn("Project Selection directory is empty!");
            return;
        }

        openProject(path);
    }

    void BosonLayer::openProject(const std::filesystem::path &path)
    {
        std::filesystem::path projectPath = path;
        if (projectPath.empty())
        {
            Log::Warn("Empty project path supplied.");
            return;
        }

        if (!m_isInitialized)
        {
            m_pendingProjectPath = projectPath;
            return;
        }

        std::error_code ec;
        if (std::filesystem::is_directory(projectPath, ec) && !ec)
        {
            auto descriptor = findProjectFileInDirectory(projectPath);
            if (descriptor.empty())
            {
                Log::Error(std::format("No .fmproj/.fproject found inside directory: {}", projectPath.string()));
                return;
            }
            projectPath = descriptor;
        }

        auto project = Project::loadProject(projectPath);
        FERMION_ASSERT(project != nullptr, "Failed to load project!");
        auto lastScene = Project::getActive()->getConfig().startScene;
        if (!lastScene.empty())
            openScene(lastScene);

        auto assetDir = Project::getActive()->getConfig().assetDirectory;
        m_contentBrowserPanel.setCurrentDirectory(assetDir);
    }

    void BosonLayer::saveProject()
    {
        saveScene();
        auto &config = Project::getActive()->getConfig();
        config.startScene = m_editorScenePath;
        if (Project::saveActive(Fermion::Project::getProjectPath()))
        {
            Log::Info("Project save successfully!");
        }
        else
        {
            Log::Error("Project save failed!");
        }
    }
} // namespace Fermion
