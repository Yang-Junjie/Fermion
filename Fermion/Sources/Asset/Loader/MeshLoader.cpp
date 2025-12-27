#include "Asset/Loader/MeshLoader.hpp"

namespace Fermion {
std::shared_ptr<Asset> MeshLoader::load(const AssetMetadata &metadata) {
    // Log::Error(metadata.FilePath.string());
    auto mesh = std::make_shared<Mesh>(metadata.FilePath.string());
    return mesh;
}
} // namespace Fermion
