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
		SceneHierarchyPanel();
		SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);
		void setContext(const std::shared_ptr<Scene> &scene);

		void onImGuiRender();

		Entity getSelectedEntity() const { return m_selectedEntity; }
		void setSelectedEntity(Entity entity);

		void setEditingEnabled(bool enabled);

	private:
		void drawEntityNode(Entity entity);
		void drawComponents(Entity entity);
		template <typename T>
		void displayAddComponentEntry(const std::string &entryName);

	private:
		std::shared_ptr<Scene> m_contextScene;
		Entity m_selectedEntity;
		bool m_editingEnabled = true;

		std::shared_ptr<Texture2D> m_spriteComponentDefaultTexture;
	};

	

}
