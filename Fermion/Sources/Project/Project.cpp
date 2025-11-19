#include "fmpch.hpp"
#include "Project.hpp"

#include "ProjectSerializer.hpp"
#include "Asset/AssetManager.hpp"

namespace Fermion {

	std::shared_ptr<Project> Project::newProject()
	{
		s_activeProject = std::make_shared<Project>();
		return s_activeProject;
	}

	std::shared_ptr<Project> Project::loadProject(const std::filesystem::path& path)
	{
		std::shared_ptr<Project> project = std::make_shared<Project>();

		ProjectSerializer serializer(project);
		if (serializer.deserialize(path))
		{
			project->m_projectDirectory = path.parent_path();
			project->m_projectPath = path;
			s_activeProject = project;
			AssetManager::init(project->getConfig().assetDirectory);
			return s_activeProject;
		}

		return nullptr;
	}

	bool Project::saveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_activeProject);
		if (serializer.serialize(path))
		{
			s_activeProject->m_projectDirectory = path.parent_path();
			s_activeProject->m_projectPath = path;
			return true;
		}

		return false;
	}

}
