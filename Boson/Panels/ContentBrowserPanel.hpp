#pragma once

#include "Renderer/Texture.hpp"

#include <filesystem>

namespace Fermion {

	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		void onImGuiRender();
	private:
		std::filesystem::path m_baseDirectory;
		std::filesystem::path m_currentDirectory;
		
		std::shared_ptr<Texture2D> m_directoryIcon = nullptr;
		std::shared_ptr<Texture2D> m_fileIcon = nullptr;
	};

}
