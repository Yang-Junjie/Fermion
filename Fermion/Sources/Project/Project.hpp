#pragma once

#include <string>
#include <memory>
#include <filesystem>


namespace Fermion {

	struct ProjectConfig
	{
		std::string name = "Untitled";

		std::filesystem::path startScene;

		std::filesystem::path assetDirectory;
	};

	class Project
	{
	public:
		static const std::filesystem::path& getProjectDirectory()
		{

			return s_activeProject->m_projectDirectory;
		}

		static std::filesystem::path getAssetDirectory()
		{
			
			return getProjectDirectory() / s_activeProject->m_config.assetDirectory;
		}

		// TODO(Yan): move to asset manager when we have one
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			return getAssetDirectory() / path;
		}

		ProjectConfig& getConfig() { return m_config; }

		static  std::shared_ptr<Project> getActive() { return s_activeProject; }

		static  std::shared_ptr<Project> newProject();
		static  std::shared_ptr<Project> loadProject(const std::filesystem::path& path);
		static bool saveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_config;
		std::filesystem::path m_projectDirectory;

		inline static std::shared_ptr<Project> s_activeProject;
	};

}
