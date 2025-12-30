#pragma once

#include "Renderer/Texture/Texture.hpp"

#include <filesystem>
#include <functional>

namespace Fermion {
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();

        void onImGuiRender();

        void setBaseDirectory(const std::filesystem::path &directory);
        void setProjectOpenCallback(std::function<void(const std::filesystem::path &)> callback);

    private:
        void drawFolderTree(const std::filesystem::path& dir);
    private:
        std::filesystem::path m_baseDirectory;
        std::filesystem::path m_currentDirectory;

        std::unique_ptr<Texture2D> m_directoryIcon = nullptr;
        std::unique_ptr<Texture2D> m_fileIcon = nullptr;
        std::unique_ptr<Texture2D> m_meshFileIcon = nullptr;
        std::unique_ptr<Texture2D> m_textureFileIcon = nullptr;
        std::function<void(const std::filesystem::path &)> m_projectOpenCallback;
    };
} // namespace Fermion
