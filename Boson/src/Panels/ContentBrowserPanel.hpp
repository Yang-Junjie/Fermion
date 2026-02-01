#pragma once

#include "Renderer/Texture/Texture.hpp"
#include "Renderer/Thumbnail/MaterialThumbnailProvider.hpp"
#include "Asset/AssetTypes.hpp"

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
        const Texture2D *getOrCreateThumbnail(const std::filesystem::path &path, AssetType type);

    private:
        std::filesystem::path m_baseDirectory;
        std::filesystem::path m_currentDirectory;

        std::unordered_map<std::string, std::unique_ptr<Texture2D>> m_thumbnailCache;

        std::unique_ptr<Texture2D> m_directoryIcon = nullptr;
        std::unique_ptr<Texture2D> m_meshFileIcon = nullptr;
        std::unique_ptr<Texture2D> m_fileIcon = nullptr;
        std::unique_ptr<Texture2D> m_textureFileIcon = nullptr;
        std::unique_ptr<Texture2D> m_MaterialFileIcon = nullptr;
        std::function<void(const std::filesystem::path &)> m_projectOpenCallback;

        std::unique_ptr<MaterialThumbnailProvider> m_materialThumbnailProvider;
    };
} // namespace Fermion
