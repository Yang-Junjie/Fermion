#pragma once
#include "AssetImporter.hpp"
#include "Renderer/Texture/Texture.hpp"

namespace Fermion {

class TextureImporter : public AssetImporter {
public:
    AssetMetadata importAsset(const std::filesystem::path &assetPath) override;

    static void generateDefaultFtex(const std::filesystem::path &imagePath);

    static void serializeFtex(const std::filesystem::path &ftexPath, const TextureAssetSpecification &spec);

    static TextureAssetSpecification deserializeFtex(const std::filesystem::path &ftexPath);

private:
    AssetMetadata m_Metadata;
};

} // namespace Fermion
