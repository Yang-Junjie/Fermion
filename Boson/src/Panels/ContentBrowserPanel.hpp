#pragma once

#include "Renderer/Texture/Texture.hpp"

#include <filesystem>
#include <functional>

namespace Fermion
{
    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void onImGuiRender();

        void setBaseDirectory(const std::filesystem::path &directory);

        void setProjectOpenCallback(std::function<void(const std::filesystem::path &)> callback);

    private:
        void drawFolderTree(const std::filesystem::path &dir);
        const Texture2D *getOrCreateThumbnail(const std::filesystem::path &path);

    private:
        std::filesystem::path m_baseDirectory;
        std::filesystem::path m_currentDirectory;

        std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_thumbnailCache;

        std::unique_ptr<Texture2D> m_directoryIcon = nullptr, m_fileIcon = nullptr, m_meshFileIcon = nullptr,
                                   m_textureFileIcon = nullptr;
        std::function<void(const std::filesystem::path &)> m_projectOpenCallback;
    };
} // namespace Fermion
