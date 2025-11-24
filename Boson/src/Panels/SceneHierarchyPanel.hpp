#pragma once
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include "InspectorPanel.hpp"

namespace Fermion
{

	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);
		void setContext(const std::shared_ptr<Scene> &scene);

		void onImGuiRender();

		Entity getSelectedEntity() const { return m_selectedEntity; }
		void setSelectedEntity(Entity entity);

		void setEditingEnabled(bool enabled);

	private:
		void drawEntityNode(Entity entity);
		// void drawComponents(Entity entity);
		// template <typename T>
		// void displayAddComponentEntry(const std::string &entryName);

	private:
		InspectorPanel m_inspectorPanel;
		std::shared_ptr<Scene> m_contextScene;
		Entity m_selectedEntity;
		bool m_editingEnabled = true;
	};

}
