#include "AssetImporter.hpp"
#include "Asset/AssetSerializer.hpp"
namespace Fermion {
void AssetImporter::writeMetadata(const AssetMetadata &metadata) {
    auto metaPath = metadata.FilePath;
    metaPath += ".meta";
    AssetSerializer::serializeMeta(metaPath, metadata);
}
AssetMetadata AssetImporter::loadMetadata(const std::filesystem::path &metaFilePath) {
    return AssetSerializer::deserializeMeta(metaFilePath);
}
} // namespace Fermion