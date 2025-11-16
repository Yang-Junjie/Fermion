#pragma once
#include "Scene/Scene.hpp"
#include "Scene/Entity.hpp"
#include <memory>
#include "Renderer/Texture.hpp"

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
		
		void setEditingEnabled(bool enabled);

	private:
		void drawEntityNode(Entity entity);
		void drawComponents(Entity entity);

	private:
		std::shared_ptr<Scene> m_contextScene;
		Entity m_selectedEntity;
		bool m_editingEnabled = true;
	};

}
