#pragma once
#include "AssetImporter.hpp"
#include "Renderer/Texture.hpp"

namespace Fermion
{
    class TextureImporter : public AssetImporter
    {
    public:
        AssetMetadata importAsset(const std::filesystem::path& assetPath) override;
    private:
        AssetMetadata m_Metadata;
    };
}
