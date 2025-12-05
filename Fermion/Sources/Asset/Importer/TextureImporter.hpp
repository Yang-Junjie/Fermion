#pragma once
#include "AssetImporter.hpp"
#include "Renderer/Texture.hpp"

namespace Fermion
{
    class TextureImporter : public AssetImporter
    {
    public:
        AssetMetadata importAsset(const std::filesystem::path& assetPath) override;
        void writeMetadata(const AssetMetadata& metadata) override;
        void loadMetadata(const std::filesystem::path& metaFilePath) override;

    private:
        AssetMetadata m_Metadata;
    };
}
