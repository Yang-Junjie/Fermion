#pragma once
#include "Renderer/Texture/Texture.hpp"
#include "Asset/Asset.hpp"
#include <filesystem>
#include <vector>
#include <string>

namespace Fermion
{
    class TextureConfigPanel
    {
    public:
        TextureConfigPanel();
        ~TextureConfigPanel() = default;

        void onImGuiRender();
        void setPanelOpenState(bool state);

        // Load single or multiple .ftex files
        void loadTexture(const std::filesystem::path &ftexPath);
        void loadTextures(const std::vector<std::filesystem::path> &ftexPaths);

        void clearData();

    private:
        struct TextureConfigData
        {
            std::filesystem::path FtexPath;
            std::string Name;
            TextureAssetSpecification Spec;
            bool Modified = false;
        };

        void drawTextureConfig(TextureConfigData &data);
        void saveTexture(TextureConfigData &data);
        void saveAllTextures();

        bool m_IsOpened = false;
        std::vector<TextureConfigData> m_Textures;
        int m_SelectedIndex = -1;

        // UI state
        bool m_BatchMode = false;
        TextureAssetSpecification m_BatchSpec;
    };
} // namespace Fermion
