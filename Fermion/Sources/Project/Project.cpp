#include "fmpch.hpp"
#include "Project.hpp"

#include "ProjectSerializer.hpp"
#include "Script/ScriptManager.hpp"
namespace Fermion
{

	std::shared_ptr<Project> Project::newProject()
	{
		s_activeProject = std::make_shared<Project>();
		return s_activeProject;
	}

	std::shared_ptr<Project> Project::loadProject(const std::filesystem::path &path)
	{
		std::shared_ptr<Project> project = std::make_shared<Project>();

		ProjectSerializer serializer(project);

		s_assetManager = std::make_unique<EditorAssetManager>();
		if (serializer.deserialize(path))
		{
			project->m_projectDirectory = path.parent_path();
			project->m_projectPath = path;
			s_activeProject = project;

			initEditorAssets();
			initRuntimeAssets();
			ScriptManager::loadScript(project->getConfig().name+".dll");

			return s_activeProject;
		}

				return nullptr;
	}

	bool Project::saveActive(const std::filesystem::path &path)
	{
		ProjectSerializer serializer(s_activeProject);
		s_assetManager = std::make_unique<EditorAssetManager>();
		if (serializer.serialize(path))
		{
			s_activeProject->m_projectDirectory = path.parent_path();
			s_activeProject->m_projectPath = path;

			initEditorAssets();
			initRuntimeAssets();

			return true;
		}

		return false;
	}

	void Project::initEditorAssets()
	{
		if (!s_activeProject)
			return;

		const auto &config = s_activeProject->getConfig();
		if (!config.assetDirectory.empty())
		{
			getEditorAssetManager()->init(config.assetDirectory);
		}
	}

	void Project::initRuntimeAssets()
	{
		if (!s_activeProject)
			return;

		const auto &config = s_activeProject->getConfig();
		if (!config.assetDirectory.empty())
		{
			getRuntimeAssetManager()->init(config.assetDirectory);
		}
	}

}
