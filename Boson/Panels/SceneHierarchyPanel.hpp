#pragma once
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include <memory>

namespace Fermion
{

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel() = default;
		SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);
		void setContext(const std::shared_ptr<Scene> &scene);

		void onImGuiRender();

		Entity getSelectedEntity() const { return m_selectedEntity; }
		void setSelectedEntity(Entity entity);

	private:
		void drawEntityNode(Entity entity);
		void drawComponents(Entity entity);

	private:
		std::shared_ptr<Scene> m_context;
		Entity m_selectedEntity;
	};

}
