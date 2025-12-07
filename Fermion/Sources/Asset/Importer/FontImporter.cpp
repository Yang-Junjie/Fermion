#include "Asset/Importer/FontImporter.hpp"
#include "Core/UUID.hpp"

namespace Fermion
{
    AssetMetadata FontImporter::importAsset(const std::filesystem::path &assetPath)
    {
        AssetMetadata metadata;

        metadata.Handle = UUID();
        if (static_cast<uint64_t>(metadata.Handle) == 0)
            metadata.Handle = UUID(1);

        metadata.Type = AssetType::Font;
        metadata.FilePath = assetPath;
        metadata.Name = assetPath.stem().string();

        return metadata;
    }
}

