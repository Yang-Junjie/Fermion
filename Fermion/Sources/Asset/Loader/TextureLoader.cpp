#include "Asset/Loader/TextureLoader.hpp"
#include "Asset/Importer/TextureImporter.hpp"

namespace Fermion
{
    std::shared_ptr<Asset> TextureLoader::load(const AssetMetadata &metadata)
    {

        TextureAssetSpecification spec = TextureImporter::deserializeFtex(metadata.FilePath);

        if (spec.SourcePath.empty()) {
            Log::Error(std::format("TextureLoader: Invalid .ftex file or missing SourcePath: {}", metadata.FilePath.string()));
            return nullptr;
        }

        if (!std::filesystem::exists(spec.SourcePath)) {
            Log::Error(std::format("TextureLoader: Source image not found: {}", spec.SourcePath.string()));
            return nullptr;
        }

        auto texture = Texture2D::create(spec);
        return texture;
    }

} // namespace Fermion
